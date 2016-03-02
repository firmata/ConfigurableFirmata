/*
  DeviceFirmata.cpp - Firmata library
*/

#include "DeviceFirmata.h"
#include <Base64.h>

extern DeviceDriver *selectedDevices[];

//----------------------------------------------------------------------------

DeviceFirmata::DeviceFirmata(const char *luRootName) {
  dt = new DeviceTable(selectedDevices, luRootName);
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
  int status = dt->dispatchTimers((ClientReporter*)this);
}

//---------------------------------------------------------------------------

// The first six bytes of argv for DEVICE_QUERY messages are: action, reserved,
// handle-low, handle-high, reserved, reserved. They are all constrained to
// 7-bit values.The bytes that follow, if any, are the parameter block. The
// parameter block is encoded with base-64 in the sysex message body during
// transmission to and from this Firmata server.

//  dpB -> decoded parameter block
//  epB -> encoded parameter block

boolean DeviceFirmata::handleSysex(byte command, byte argc, byte *argv) {
      char *errString;

  byte dpBlock[1 + MAX_DPB_LENGTH]; // decoded parameter block

  if (command != DEVICE_QUERY) {
    return false;
  }

  int action = argv[0];
  int handle = (argv[3] << 7) | argv[2];

  int dpCount = base64_dec_len((char *)(argv + 6), argc - 6);
  if (dpCount > MAX_DPB_LENGTH) {
    sendDeviceResponse(action, EMSGSIZE, handle);
    return true;
  }

  if (dpCount > 0) {
    dpCount = base64_decode((char *)dpBlock, (char *)(argv + 6), argc - 6);
  }

  int flags = 0;
  int status = 0;
  int reg = 0;
  int count = 0;

  switch (action) {

  case DD_OPEN:
    flags = handle;
    status = dt->open((char *)dpBlock, flags);
    reportOpen(status);
    break;

  case DD_READ:
    reg   = (int8_t)from8LEToHost(dpBlock);
    count = from16LEToHost(dpBlock + 1);
    status = dt->read(handle, reg, count, dpBlock + 3);
    reportRead(status, handle, dpBlock);
    break;

  case DD_WRITE:
    reg   = (int8_t)from8LEToHost(dpBlock);
    count = from16LEToHost(dpBlock + 1);
    status = dt->write(handle, reg, count, dpBlock + 3);
    reportWrite(status, handle, dpBlock);
    break;

  case DD_CLOSE:
    status = dt->close(handle);
    reportClose(status, handle);
    break;

  default:
    sendDeviceResponse(action, ENOSYS, handle);
    break;

  }
  return true;
}

//---------------------------------------------------------------------------

  // in all cases, open and close don't return any bytes in the parameter buffer
  // on error, both read and write return 3 bytes in the parameter buffer
  // on read success, 3 bytes plus the data read are returned in the parameter buffer
  // on write success, 3 bytes are returned in the parameter buffer

void DeviceFirmata::reportOpen(int status) {
  sendDeviceResponse(DD_OPEN, status, 0);
}
/**
 * Translates a message from the DeviceDriver environment to a call to a Firmata-aware method.
 * @param status The status code or actual byte count associated with this read.
 * @param handle The 14-bit handle of the unit doing the reply
 * @param dpB    The byte buffer holding the reg, requested byte count and the data that was read
 */
  void DeviceFirmata::reportRead(int status, int handle, const byte *dpB) {
  int dpCount = (status >= 0) ? status + 3 : 3;
  sendDeviceResponse(DD_READ, status, handle, dpCount, dpB);
}
/**
 * Translates a message from the DeviceDriver environment to a call to a Firmata-aware method.
 * @param status The status code or actual byte count associated with this write.
 * @param handle The 14-bit handle of the unit doing the reply
 * @param dpB    The byte buffer holding the reg and requested byte count
 */
void DeviceFirmata::reportWrite(int status, int handle, const byte *dpB) {
  int dpCount = 3;
  sendDeviceResponse(DD_WRITE, status, handle, dpCount, dpB);
}

void DeviceFirmata::reportClose(int status, int handle) {
  sendDeviceResponse(DD_CLOSE, status, handle);
}

//---------------------------------------------------------------------------

/**
 * This method is called when there is a message to be sent back to the
 * client.  It may be in response to a DEVICE_REQUEST that was just
 * processed, or it may be an asynchronous event such as a stepper motor in
 * a new position or a continuous read data packet.
 * @param action  The method identifier to use in the response.
 * @param status  Status value to send or number of bytes actually read or written
 * @param handle  The 14-bit handle identifying the device and unit number the message is coming from
 * @param dpCount The number of bytes to be encoded from the dpB and sent on
 * @param dpBlock The decoded (raw) parameter block to send upwards after encoding
 */
void DeviceFirmata::sendDeviceResponse(int action, int status, int handle, int dpCount, const byte *dpB) {
  byte epB[1 + 4 + ((MAX_DPB_LENGTH + 2) / 3) * 4];
  byte header[8] = {START_SYSEX, DEVICE_RESPONSE};
  int epCount = 0;

  header[2] = (byte) action;
  header[3] = (byte) 0;
  header[4] = (byte)getUnitNumber(handle);
  header[5] = (byte)getDeviceNumber(handle);
  header[6] = (byte)(status & 0x7F);
  header[7] = (byte)((status >> 7) & 0x7F);

  for (int idx = 0; idx < 8; idx++) {
    Firmata.write(header[idx]);
  }

//  dpB -> decoded parameter block
//  epB -> encoded parameter block

  if (dpCount > 0 && dpCount <= MAX_DPB_LENGTH && dpB != 0) {
    epCount = base64_encode((char *)epB, (char *)dpB, dpCount);
  }

  for (int idx = 0; idx < epCount; idx++) {
    Firmata.write(epB[idx]);
  }
  Firmata.write(END_SYSEX);
}
