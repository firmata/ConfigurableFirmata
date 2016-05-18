/*
  DeviceFirmata.cpp - Firmata library
*/

#include "DeviceFirmata.h"
#include "utility/Boards.h"
#include <Base64.h>

DeviceTable *gDeviceTable;
extern DeviceDriver *selectedDevices[];

//----------------------------------------------------------------------------

DeviceFirmata::DeviceFirmata() {
  gDeviceTable = new DeviceTable(selectedDevices,this);
}

//---------------------------------------------------------------------------

void DeviceFirmata::reset() {
  gDeviceTable->reset();
}

// The pins that are currently being used by device drivers are already
// marked as PIN_MODE_IGNORE, so we have no additional information to
// provide to the list of capabilities.

void DeviceFirmata::handleCapability(byte pin) {}

// Device driver capabilities do not necessarily map directly to Firmata pin
// modes, and so none of the Firmata modes are recognized and all pin mode
// requests are disavowed.

boolean DeviceFirmata::handlePinMode(byte pin, int mode) {
  return false;
}

void DeviceFirmata::update() {
  gDeviceTable->dispatchTimers();
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

  int action = (from8LEToHost(parameterBlock) & 0x0F);
  int flags  = (from8LEToHost(parameterBlock) & 0xF0) >> 4;
  int handle = from16LEToHost(parameterBlock+1);
  int openOpts = handle;
  int reg    = from16LEToHost(parameterBlock+3);
  int count  = from16LEToHost(parameterBlock+5);

  int status;

  switch (action) {

  case (int)DAC::OPEN:
    if (dataBlockLength == 0) {
      reportError(EINVAL);
    } else {
      status = gDeviceTable->open(openOpts, flags, (const char *)dataBlock);
      reportOpen(status, openOpts, flags, dataBlock);
    }
    break;

  case (int)DAC::READ:
    if (dataBlockLength != 0) {
      reportError(EINVAL);
    } else {
      inputBuffer = new byte[count];
      if (inputBuffer == 0) {
        reportError(ENOMEM);
      } else {
        status = gDeviceTable->read(handle, flags, reg, count, inputBuffer);
        reportRead(status, handle, flags, reg, count, inputBuffer);
      }
    }
    break;

  case (int)DAC::WRITE:
    if (dataBlockLength != count) {
      reportError(EINVAL);
    } else {
      status = gDeviceTable->write(handle, flags, reg, count, dataBlock);
      reportWrite(status, handle, flags, reg, count);
    }
    break;

  case (int)DAC::CLOSE:
    if (dataBlockLength != 0) {
      reportError(EINVAL);
    } else {
      status = gDeviceTable->close(handle, flags);
      reportClose(status, handle, flags);
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

void reportPinClaim(int pin) {
  Firmata.setPinMode((byte)pin, PIN_MODE_IGNORE);
}

void reportPinRelease(int pin) {
  Firmata.setPinMode((byte)pin, INPUT);
}

void DeviceFirmata::reportOpen(int status, int openOpts, int flags, const byte *buf) {
  sendDeviceResponse((int)DAC::OPEN, status, openOpts, flags, 0, 0,buf);
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
void DeviceFirmata::reportRead(int status, int handle, int flags, int reg, int count, const byte *buf) {
  sendDeviceResponse((int)DAC::READ, status, handle, flags, reg, count, buf);
}
/**
 * Translates a message from the DeviceDriver environment to a call to a Firmata-aware method.
 * @param status The status code or actual byte count associated with this write.
 * @param handle The handle of the unit doing the reply
 * @param reg The register identifier associated with the write() being reported
 * @param count The number of bytes that were requested.  May be less than or
 * equal to the byte count in status after a successful write.
 */
void DeviceFirmata::reportWrite(int status, int handle, int flags, int reg, int count) {
  sendDeviceResponse((int)DAC::WRITE, status, handle, flags, reg, count);
}

void DeviceFirmata::reportClose(int status, int handle, int flags) {
  sendDeviceResponse((int)DAC::CLOSE, status, handle, flags);
}

void DeviceFirmata::reportError(int status) {
  sendDeviceResponse((int)DAC::CLOSE, status);
}

void DeviceFirmata::reportString(const byte *dataBytes) {
  Firmata.sendString((char *)dataBytes);
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
void DeviceFirmata::sendDeviceResponse(int action, int status, int handle, int flags, int reg, int count,
                                       const byte *dataBytes) {

  byte dP[9];       // decoded (raw) message prologue
  byte eP[12+1];    // encoded message prologue
  byte *eD;         // encoded data bytes

  Firmata.write(START_SYSEX);
  Firmata.write(DEVICE_RESPONSE);

  dP[0] = (byte) ((flags & 0xF) << 4) | (action & 0xF);
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

  int rawCount = 0;
  int encCount = 0;

  if (dataBytes != 0) {
    if (action == (int)DAC::OPEN) {
      rawCount = strlen((const char *)dataBytes)+1;
    } else if (action == (int)DAC::READ && status > 0) {
      rawCount = status;
    }
    encCount = base64_enc_len(rawCount);
    if (encCount > 0) {
      eD = new byte[encCount+1];
      if (eD == 0) {
        for (int idx = 0; idx < encCount; idx++) {
          Firmata.write('/');     // Memory allocation error.  This value will be decoded as 0x3F, ie, all 7 bits set.
        }
      } else {
        base64_encode((char *)eD, (char *)dataBytes, rawCount);
        for (int idx = 0; idx < encCount; idx++) {
          Firmata.write(eD[idx]);     // Success.  These are the encoded data bytes.
        }
      }
      delete eD;
    }
  }
  Firmata.write(END_SYSEX);
}
