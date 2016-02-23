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
  dt->dispatchTimers((ClientReporter*)this);
}

// The first six bytes of argv for DEVICE_QUERY messages are: action, reserved,
// handle-low, handle-high, reserved, reserved. They are all constrained to
// 7-bit values.The bytes that follow, if any, are the parameter block. The
// parameter block is encoded with base-64 in the sysex message body during
// transmission to and from this Firmata server.

//  dpB -> decoded parameter block
//  epB -> encoded parameter block

boolean DeviceFirmata::handleSysex(byte command, byte argc, byte *argv) {

  byte dpBlock[1 + MAX_DPB_LENGTH]; // decoded parameter block

  if (command != DEVICE_QUERY) {
    return false;
  }

  int action = argv[0];
  int handle = (argv[3] << 7) | argv[2];

  int dpCount = base64_dec_len((char *)(argv + 6), argc - 6);
  if (dpCount > MAX_DPB_LENGTH) {
    sendDeviceResponse(handle, action, EMSGSIZE);
    return true;
  }

  if (dpCount > 0) {
    dpCount = base64_decode((char *)dpBlock, (char *)(argv + 6), argc - 6);
  }

  dispatchDeviceAction(handle, action, dpCount, dpBlock);
  return true;
}

//---------------------------------------------------------------------------

void DeviceFirmata::dispatchDeviceAction(int handle, int action, int dpCount, byte *dpBlock) {
  int status = 0;
  int count = 0;
  int reg = 0;
  int flags = 0;

  switch (action) {
  case DD_OPEN:
    flags = handle;
    status = dt->open((char *)dpBlock, flags);
    reportOpen(status);
    break;

  case DD_STATUS:
    count = from16LEToHost(dpBlock);
    reg   = from16LEToHost(dpBlock + 2);
    status = dt->status(handle, reg, count, dpBlock);
    reportStatus(handle, status, dpBlock);
    break;

  case DD_CONTROL:
    count = from16LEToHost(dpBlock);
    reg   = from16LEToHost(dpBlock + 2);
    status = dt->control(handle, reg, count, dpBlock + 4);
    reportControl(handle, status);
    break;

  case DD_READ:
    count = from16LEToHost(dpBlock);
    status = dt->read(handle, count, dpBlock);
    reportRead(handle, status, dpBlock);
    break;

  case DD_WRITE:
    count = from16LEToHost(dpBlock);
    status = dt->read(handle, count, dpBlock + 2);
    reportWrite(handle, status);
    break;

  case DD_CLOSE:
    status = dt->close(handle);
    reportClose(handle, status);
    break;

  default:
    status = ENOSYS;
    break;
  }
}

//---------------------------------------------------------------------------

/**
 * This method is called when there is a message to be sent back to the
 * client.  It may be in response to a DEVICE_REQUEST that was just
 * processed, or it may be an asynchronous event such as a stepper motor in
 * a new position or a continuous read data packet.
 * @param handle  The 14-bit handle identifying the device and unit
 * number the message is coming from
 * @param action  The method identifier to use in the response.
 * @param status  Status value to send or number of bytes in dpBlock to send
 * @param dpBlock The decoded (raw) parameter block to send upwards.
 */
void DeviceFirmata::sendDeviceResponse(int handle, int action, int status, const byte *dpB) {
  byte epB[1 + ((MAX_DPB_LENGTH + 2) / 3) * 4];
  byte header[8] = {START_SYSEX, DEVICE_RESPONSE};
  int epCount = 0;

  header[2] = (byte) action;
  header[3] = (byte) 0;
  header[4] = (byte)getUnitHandle(handle);
  header[5] = (byte)getDeviceHandle(handle);
  header[6] = (byte)(status & 0x7F);
  header[7] = (byte)((status >> 7) & 0x7F);

  for (int idx = 0; idx < 8; idx++) {
    Firmata.write(header[idx]);
  }

//  dpB -> decoded parameter block
//  epB -> encoded parameter block

  if (status > 0 && status <= MAX_DPB_LENGTH && dpB != 0) {
    epCount = base64_encode((char *)epB, (char *)dpB, status);
  }

  for (int idx = 0; idx < epCount; idx++) {
    Firmata.write(epB[idx]);
  }
  Firmata.write(END_SYSEX);
}

//---------------------------------------------------------------------------

void DeviceFirmata::reportOpen(int status) {
  sendDeviceResponse(DD_OPEN, 0, status);
}

void DeviceFirmata::reportStatus(int handle, int status, const byte *dpB) {
  if (status < 0) {
    sendDeviceResponse(DD_STATUS, handle, status);
  } else {
    sendDeviceResponse(DD_STATUS, handle, status, dpB);
  }
}

void DeviceFirmata::reportRead(int handle, int status, const byte *dpB) {
  if (status < 0) {
    sendDeviceResponse(DD_READ, handle, status);
  } else {
    sendDeviceResponse(DD_READ, handle, status, dpB);
  }
}

void DeviceFirmata::reportControl(int handle, int status) {
  sendDeviceResponse(DD_CONTROL, handle, status);
}

void DeviceFirmata::reportWrite(int handle, int status) {
  sendDeviceResponse(DD_WRITE, handle, status);
}

void DeviceFirmata::reportClose(int handle, int status) {
  sendDeviceResponse(DD_CLOSE, handle, status);
}

