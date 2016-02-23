
#ifndef DeviceFirmata_h
#define DeviceFirmata_h

#include <FirmataFeature.h>
#include <LuniLib.h>
#include <Device/DeviceDriver.h>
#include <Device/DeviceTable.h>
#include <Device/ClientReporter.h>

#define MAX_DPB_LENGTH 128  // decoded parameter block length (plain text)

// Firmata coding of DeviceDriver methods

#define  DD_OPEN    0x00
#define  DD_STATUS  0x01
#define  DD_CONTROL 0x02
#define  DD_READ    0x03
#define  DD_WRITE   0x04
#define  DD_CLOSE   0x05

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
    void reportStatus(int handle, int status, const byte *dpB);
    void reportRead(int handle, int status, const byte *dpB);
    void reportControl(int handle, int status);
    void reportWrite(int handle, int status);
    void reportClose(int handle, int status);

private:
    DeviceTable *dt;

    void dispatchDeviceAction(int handle, int act, int pc, byte *pv);
    void sendDeviceResponse(int handle, int action, int status, const byte *dpBlock = 0);
};

#endif

