
#ifndef DeviceFirmata_h
#define DeviceFirmata_h

#include <FirmataFeature.h>
#include <LuniLib.h>
#include <Device/DeviceDriver.h>
#include <Device/DeviceTable.h>
#include <Device/ClientReporter.h>

#define MAX_READ_COUNT 150  // max number of bytes that can be read with one message

// Firmata coding of DeviceDriver methods

#define  DD_OPEN    0x00
#define  DD_READ    0x01
#define  DD_WRITE   0x02
#define  DD_CLOSE   0x03

class DeviceFirmata: public FirmataFeature, ClientReporter {
public:
    DeviceFirmata(const char *luRootName = 0);

    // FirmataFeature

    void reset();
    void handleCapability(byte pin);
    boolean handlePinMode(byte pin, int mode);
    boolean handleSysex(byte command, byte argc, byte* argv);

    void update();

    // ClientReporter

    void reportOpen(int status);
    void reportRead(int status, int handle, int reg, int count, const byte *dataBytes);
    void reportWrite(int status, int handle, int reg, int count);
    void reportClose(int status, int handle);
    void reportError(int status);

private:
    DeviceTable *dt;

    void sendDeviceResponse(int action, int status, int handle = 0, int reg = 0, int count = 0, const byte *dataBytes = 0);
};

#endif

