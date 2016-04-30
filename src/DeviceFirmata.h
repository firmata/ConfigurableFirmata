
#ifndef DeviceFirmata_h
#define DeviceFirmata_h

#include <FirmataFeature.h>
#include <LuniLib.h>
#include <Device/DeviceDriver.h>
#include <Device/DeviceTable.h>
#include <Device/ClientReporter.h>

class DeviceFirmata: public FirmataFeature, ClientReporter {
public:
    DeviceFirmata();

    // FirmataFeature

    void reset();
    void handleCapability(byte pin);
    boolean handlePinMode(byte pin, int mode);
    boolean handleSysex(byte command, byte argc, byte* argv);

    void update();

    // ClientReporter

    void reportOpen(int status, int opts, int flags, const byte *buf);
    void reportRead(int status, int handle, int flags, int reg, int count, const byte *dataBytes);
    void reportWrite(int status, int handle, int flags,  int reg, int count);
    void reportClose(int status, int handle, int flags );
    void reportError(int status);

private:

    void sendDeviceResponse(int action, int status, int handle = 0, int flags = 0, int reg = 0, int count = 0, const byte *dataBytes = 0);
};

#endif

