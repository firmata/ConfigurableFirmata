/*
  DeviceFirmata.cpp - Firmata library
*/

#include "DeviceFirmata.h"
#include <Base64.h>

extern DeviceDriver *selectedDevices[];

//----------------------------------------------------------------------------

DeviceFirmata::DeviceFirmata() {
  dt = new DeviceTable(selectedDevices);
}

//---------------------------------------------------------------------------

void DeviceFirmata::reset() {
  dt->reset();
}

void DeviceFirmata::handleCapability(byte pin) {}

boolean DeviceFirmata::handlePinMode(byte pin, int mode) {
  return false;
}

void DeviceFirmata::update() {
  dt->dispatchTimers((ClientReporter*)this);
}

//---------------------------------------------------------------------------

// The entire body of each device driver message is encoded in base-64
// during transmission to and from this Firmata server.  The first 9 bytes
// of the decoded message body form a prologue that contains slots for all
// the common query and request parameters.  Following bytes in a query
// are used by the open() and write() methods; following bytes in a response
// are used by the read() method.

// Note that the base64 library adds a null terminator after the decoded data.
// This means that the decode targets need to be at least one byte bigger than
// the actual data size.  This also makes it possible to use the decoded data
// as a null-terminated string without having to add the null (eg, open())

boolean DeviceFirmata::handleSysex(byte command, byte argc, byte *argv) {
  if (command != DEVICE_QUERY) return false;

  if (argc < 12) {
    reportError(EMSGSIZE);
    return true;
  }
  byte parameterBlock[10];  // 9-byte beginning of every decoded DEVICE_QUERY message
  byte *dataBlock = 0;      // data from the client for use in open() and write()
  byte *inputBuffer = 0;    // data from the device in response to read()

  base64_decode((char *)parameterBlock, (char *)(argv), 12);

  int dataBlockLength = (argc == 12) ? 0 : base64_dec_len((char *)(argv + 12), argc - 12);
  if (dataBlockLength > 0) {
    dataBlock =  new byte[dataBlockLength+1];
    if (dataBlock == 0) {
      reportError(ENOMEM);
      return true;
    }
    dataBlockLength = base64_decode((char *)dataBlock, (char *)(argv + 12), argc - 12);
  }

  int action = from8LEToHost(parameterBlock);
  int handle = from16LEToHost(parameterBlock+1);
  int reg    = from16LEToHost(parameterBlock+3);
  int count  = from16LEToHost(parameterBlock+5);

  int flags = 0;
  int status = 0;

  switch (action) {

  case DD_OPEN:
    if (dataBlockLength == 0) {
      reportError(EINVAL);
    } else {
      flags = handle;
      status = dt->open((const char *)dataBlock, flags);
      reportOpen(status);
    }
    break;

  case DD_READ:
    if (dataBlockLength != 0) {
      reportError(EINVAL);
    } else {
      inputBuffer = new byte[count];
      if (inputBuffer == 0) {
        reportError(ENOMEM);
      } else {
        status = dt->read(handle, reg, count, inputBuffer);
        reportRead(status, handle, reg, count, inputBuffer);
      }
    }
    break;

  case DD_WRITE:
    if (dataBlockLength != count) {
      reportError(EINVAL);
    } else {
      status = dt->write(handle, reg, count, dataBlock);
      reportWrite(status, handle, reg, count);
    }
    break;

  case DD_CLOSE:
    if (dataBlockLength != 0) {
      reportError(EINVAL);
    } else {
      status = dt->close(handle);
      reportClose(status, handle);
    }
    break;

  default:
    reportError(ENOTSUP);
    break;
  }

  delete dataBlock;
  delete inputBuffer;
  return true;
}

//---------------------------------------------------------------------------

void DeviceFirmata::reportOpen(int status) {
  sendDeviceResponse(DD_OPEN, status);
}
/**
 * Translates a message from the DeviceDriver environment to a call to a Firmata-aware method.
 * @param status The status code or actual byte count associated with this read.
 * @param handle The handle of the unit doing the reply
 * @param reg The register identifier associated with the read() being reported
 * @param count The number of bytes that were requested.  May be less than or
 * equal to the byte count in status after a successful read.
 * @param buf The byte[] result of the read().
 */
void DeviceFirmata::reportRead(int status, int handle, int reg, int count, const byte *buf) {
  sendDeviceResponse(DD_READ, status, handle, reg, count, buf);
}
/**
 * Translates a message from the DeviceDriver environment to a call to a Firmata-aware method.
 * @param status The status code or actual byte count associated with this write.
 * @param handle The handle of the unit doing the reply
 * @param reg The register identifier associated with the write() being reported
 * @param count The number of bytes that were requested.  May be less than or
 * equal to the byte count in status after a successful write.
 */
void DeviceFirmata::reportWrite(int status, int handle, int reg, int count) {
  sendDeviceResponse(DD_WRITE, status, handle, reg, count);
}

void DeviceFirmata::reportClose(int status, int handle) {
  sendDeviceResponse(DD_CLOSE, status, handle);
}

void DeviceFirmata::reportError(int status) {
  sendDeviceResponse(DD_CLOSE, status);
}

//---------------------------------------------------------------------------

/**
 * This method is called when there is a message to be sent back to the
 * client.  It may be in response to a DEVICE_REQUEST that was just
 * processed, or it may be an asynchronous event such as a stepper motor in
 * a new position or a continuous read data packet.
 *
 *  dmB -> decoded message body
 *  emB -> encoded message body
 *
 * @param action  The method identifier to use in the response.
 * @param status  Status value, new handle (open), or number of bytes actually read or written
 * @param handle  The handle identifying the device and unit number the message is coming from
 * @param reg The register number associated with this message
 * @param count The number of bytes specified originally by the caller
 * @param dataBytes The raw data read from the device, if any
 *
 * Note:  The base64 encoder adds a null at the end of the encoded data.  Thus the encode
 * target buffer needs to be one byte longer than the calculated data length.
 */
void DeviceFirmata::sendDeviceResponse(int action, int status, int handle, int reg, int count,
                                       const byte *dataBytes) {

  byte dP[9];       // source (raw) message prologue
  byte eP[12+1];    // encoded message prologue
  byte *eD;         // encoded data bytes

  Firmata.write(START_SYSEX);
  Firmata.write(DEVICE_RESPONSE);

  dP[0] = (byte) action;
  dP[1] = (byte) lowByte(handle);
  dP[2] = (byte) highByte(handle);
  dP[3] = (byte) lowByte(reg);
  dP[4] = (byte) highByte(reg);
  dP[5] = (byte) lowByte(count);
  dP[6] = (byte) highByte(count);
  dP[7] = (byte) lowByte(status);
  dP[8] = (byte) highByte(status);

  base64_encode((char *)eP, (char *)dP, 9);

  for (int idx = 0; idx < 12; idx++) {
    Firmata.write(eP[idx]);
  }

  if (action == DD_READ && status > 0) {
    int eDCount = base64_enc_len(status);
    eD = new byte[eDCount+1];
    if (dataBytes == 0 || eD == 0) {
      for (int idx = 0; idx < eDCount; idx++) {
        Firmata.write('/');     // Error.  This value will be decoded as 0x3F, ie, all 6 bits set.
      }
    } else {
      base64_encode((char *)eD, (char *)dataBytes, status);
      for (int idx = 0; idx < eDCount; idx++) {
        Firmata.write(eD[idx]);     // Success.  These are the encoded data bytes.
      }
    }
    delete eD;
  }
  Firmata.write(END_SYSEX);
}
