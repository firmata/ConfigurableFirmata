/*
  Firmata is a generic protocol for communicating with microcontrollers
  from software on a host computer. It is intended to work with
  any host computer software package.

  To download a host software package, please clink on the following link
  to open the download page in your default browser.

  https://github.com/firmata/ConfigurableFirmata#firmata-client-libraries

  Copyright (C) 2006-2008 Hans-Christoph Steiner.  All rights reserved.
  Copyright (C) 2010-2011 Paul Stoffregen.  All rights reserved.
  Copyright (C) 2009 Shigeru Kobayashi.  All rights reserved.
  Copyright (C) 2013 Norbert Truchsess. All rights reserved.
  Copyright (C) 2014 Nicolas Panel. All rights reserved.
  Copyright (C) 2015-2016 Jesse Frush. All rights reserved.
  Copyright (C) 2009-2017 Jeff Hoefs.  All rights reserved.
  Copyright (C) 2016 Jens B. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

  Last updated: September 16th, 2017
*/

/*
  README

  This is an example use of ConfigurableFirmata with WiFi.

  ConfigurableFirmataWiFi enables the use of Firmata over a TCP connection. It can be configured as
  either a TCP server or TCP client. To configure your Wi-Fi connection, follow the instructions in
  the WIFI CONFIGURATION section of this file.

  To use ConfigurableFirmataWiFi you will need to have one of the following
  boards or shields:

  - Arduino MKR1000 board (recommended)
  - ESP8266 WiFi board compatible with ESP8266 Arduino core
  - Arduino WiFi Shield 101
  - Arduino WiFi Shield (or clone)

  Follow the instructions in the wifiConfig.h file (wifiConfig.h tab in Arduino IDE) to
  configure your particular hardware.

  Dependencies:
  - WiFi Shield 101 requires version 0.7.0 or higher of the WiFi101 library (available in Arduino
    1.6.8 or higher, or update the library via the Arduino Library Manager or clone from source:
    https://github.com/arduino-libraries/WiFi101)
  - ESP8266 requires the Arduino ESP8266 core v2.1.0 or higher which can be obtained here:
    https://github.com/esp8266/Arduino

  In order to use the WiFi Shield 101 with Firmata you will need a board with at least 35k of Flash
  memory. This means you cannot use the WiFi Shield 101 with an Arduino Uno or any other
  ATmega328p-based microcontroller or with an Arduino Leonardo or other ATmega32u4-based
  microcontroller. Some boards that will work are:

  - Arduino Zero
  - Arduino Due
  - Arduino 101
  - Arduino Mega

  NOTE: If you are using an Arduino WiFi (legacy) shield you cannot use the following pins on
  the following boards. Firmata will ignore any requests to use these pins:

  - Arduino Uno or other ATMega328 boards: (D4, D7, D10, D11, D12, D13)
  - Arduino Mega: (D4, D7, D10, D50, D51, D52, D53)
  - Arduino Due, Zero or Leonardo: (D4, D7, D10)

  If you are using an Arduino WiFi 101 shield you cannot use the following pins on the following
  boards:

  - Arduino Due or Zero: (D5, D7, D10)
  - Arduino Mega: (D5, D7, D10, D50, D52, D53)
*/

#include "ConfigurableFirmata.h"

/*
 * Uncomment the #define SERIAL_DEBUG line below to receive serial output messages relating to your
 * connection that may help in the event of connection issues. If defined, some boards may not begin
 * executing this sketch until the Serial console is opened.
 */
//#define SERIAL_DEBUG
#include "utility/firmataDebug.h"

#define MAX_CONN_ATTEMPTS 20  // [500 ms] -> 10 s

/*==============================================================================
 * WIFI CONFIGURATION
 *
 * You must configure your particular hardware. Follow the steps below.
 *
 * By default, ConfigurableFirmataWiFi is configured as a TCP server, to configure
 * as a TCP client, see STEP 2.
 *============================================================================*/

// STEP 1 [REQUIRED]
// Uncomment / comment the appropriate set of includes for your hardware (OPTION A, B or C)
// Arduino MKR1000 or ESP8266 are enabled by default if compiling for either of those boards.

/*
 * OPTION A: Configure for Arduino MKR1000 or Arduino WiFi Shield 101
 *
 * This will configure ConfigurableFirmataWiFi to use the WiFi101 library, which works with the
 * Arduino WiFi101 shield and devices that have the WiFi101 chip built in (such as the MKR1000).
 * It is compatible with 802.11 B/G/N networks.
 *
 * If you are using the MKR1000 board, continue on to STEP 2. If you are using the WiFi 101 shield,
 * follow the instructions below.
 *
 * To enable for the WiFi 101 shield, uncomment the #define WIFI_101 below and verify the
 * #define ARDUINO_WIFI_SHIELD is commented out for OPTION B.
 *
 * IMPORTANT: You must have the WiFI 101 library installed. To easily install this library, open
 * the library manager via: Arduino IDE Menus: Sketch > Include Library > Manage Libraries > filter
 * search for "WiFi101" > Select the result and click 'install'
 */
//#define WIFI_101

//do not modify the following 11 lines
#if defined(ARDUINO_SAMD_MKR1000) && !defined(WIFI_101)
// automatically include if compiling for MRK1000
#define WIFI_101
#endif
#ifdef WIFI_101
#include <WiFi101.h>
#include "utility/WiFiClientStream.h"
#include "utility/WiFiServerStream.h"
  #define WIFI_LIB_INCLUDED
#endif

/*
 * OPTION B: Configure for legacy Arduino WiFi shield
 *
 * This will configure ConfigurableFirmataWiFi to use the original WiFi library (deprecated) provided
 * with the Arduino IDE. It is supported by the Arduino WiFi shield (a discontinued product) and
 * is compatible with 802.11 B/G networks.
 *
 * To configure ConfigurableFirmataWiFi to use the legacy Arduino WiFi shield
 * leave the #define below uncommented and ensure #define WIFI_101 is commented out for OPTION A.
 */
//#define ARDUINO_WIFI_SHIELD

//do not modify the following 10 lines
#ifdef ARDUINO_WIFI_SHIELD
#include <WiFi.h>
#include "utility/WiFiClientStream.h"
#include "utility/WiFiServerStream.h"
  #ifdef WIFI_LIB_INCLUDED
  #define MULTIPLE_WIFI_LIB_INCLUDES
  #else
  #define WIFI_LIB_INCLUDED
  #endif
#endif

/*
 * OPTION C: Configure for ESP8266
 *
 * This will configure ConfigurableFirmataWiFi to use the ESP8266WiFi library for boards
 * with an ESP8266 chip. It is compatible with 802.11 B/G/N networks.
 *
 * The appropriate libraries are included automatically when compiling for the ESP8266 so
 * continue on to STEP 2.
 *
 * IMPORTANT: You must have the esp8266 board support installed. To easily install this board see
 * the instructions here: https://github.com/esp8266/Arduino#installing-with-boards-manager.
 */
//do not modify the following 14 lines
#ifdef ESP8266
// automatically include if compiling for ESP8266
#define ESP8266_WIFI
#endif
#ifdef ESP8266_WIFI
#include <ESP8266WiFi.h>
#include "utility/WiFiClientStream.h"
#include "utility/WiFiServerStream.h"
  #ifdef WIFI_LIB_INCLUDED
  #define MULTIPLE_WIFI_LIB_INCLUDES
  #else
  #define WIFI_LIB_INCLUDED
  #endif
#endif


// STEP 2 [OPTIONAL for all boards and shields]
// By default the board/shield is configured as a TCP server.
// If you want to setup you board/shield as a TCP client, uncomment the following define and
// replace the REMOTE_SERVER_IP address below with the IP address of your remote server.
//#define REMOTE_SERVER_IP 10, 0, 0, 15


// STEP 3 [REQUIRED for all boards and shields]
// replace this with your wireless network SSID
char ssid[] = "your_network_name";


// STEP 4 [OPTIONAL for all boards and shields]
// If you want to use a static IP (v4) address, uncomment the line below. You can also change the IP.
// If the first line is commented out, the WiFi shield will attempt to get an IP from the DHCP server.
// If you are using a static IP with the ESP8266 then you must also uncomment the SUBNET and GATEWAY.
//#define STATIC_IP_ADDRESS  192,168,1,113
//#define SUBNET_MASK        255,255,255,0 // REQUIRED for ESP8266_WIFI, optional for others
//#define GATEWAY_IP_ADDRESS 0,0,0,0       // REQUIRED for ESP8266_WIFI, optional for others


// STEP 5 [REQUIRED for all boards and shields]
// define your port number here, you will need this to open a TCP connection to your Arduino
#define NETWORK_PORT 3030


// STEP 6 [REQUIRED for all boards and shields]
// determine your network security type (OPTION A, B, or C). Option A is the most common, and the
// default.

/*
 * OPTION A: WPA / WPA2
 *
 * WPA is the most common network security type. A passphrase is required to connect to this type.
 *
 * To enable, leave #define WIFI_WPA_SECURITY uncommented below, set your wpa_passphrase value
 * appropriately, and do not uncomment the #define values under options B and C
 */
#define WIFI_WPA_SECURITY

#ifdef WIFI_WPA_SECURITY
char wpa_passphrase[] = "your_wpa_passphrase";
#endif  //WIFI_WPA_SECURITY


/*
 * OPTION B: WEP (not supported for ESP8266)
 *
 * WEP is a less common (and regarded as less safe) security type. A WEP key and its associated
 * index are required to connect to this type.
 *
 * To enable, Uncomment the #define below, set your wep_index and wep_key values appropriately,
 * and verify the #define values under options A and C are commented out.
 */
//#define WIFI_WEP_SECURITY

#ifdef WIFI_WEP_SECURITY
//The wep_index below is a zero-indexed value.
//Valid indices are [0-3], even if your router/gateway numbers your keys [1-4].
byte wep_index = 0;
char wep_key[] = "your_wep_key";
#endif  //WIFI_WEP_SECURITY


/*
 * OPTION C: Open network (no security)
 *
 * Open networks have no security, can be connected to by any device that knows the ssid, and are
 * unsafe.
 *
 * To enable, uncomment #define WIFI_NO_SECURITY below and verify the #define values
 * under options A and B are commented out.
 */
//#define WIFI_NO_SECURITY

/*==============================================================================
 * CONFIGURATION ERROR CHECK (don't change anything here)
 *============================================================================*/

#ifdef MULTIPLE_WIFI_LIB_INCLUDES
#error "you may not define more than one wifi device type in wifiConfig.h."
#endif

#ifndef WIFI_LIB_INCLUDED
#error "you must define a wifi device type in wifiConfig.h."
#endif

#if ((defined(WIFI_NO_SECURITY) && (defined(WIFI_WEP_SECURITY) || defined(WIFI_WPA_SECURITY))) || (defined(WIFI_WEP_SECURITY) && defined(WIFI_WPA_SECURITY)))
#error "you may not define more than one security type at the same time in wifiConfig.h."
#endif  //WIFI_* security define check

#if !(defined(WIFI_NO_SECURITY) || defined(WIFI_WEP_SECURITY) || defined(WIFI_WPA_SECURITY))
#error "you must define a wifi security type in wifiConfig.h."
#endif  //WIFI_* security define check

#if (defined(ESP8266_WIFI) && !(defined(WIFI_NO_SECURITY) || (defined(WIFI_WPA_SECURITY))))
#error "you must choose between WIFI_NO_SECURITY and WIFI_WPA_SECURITY"
#endif

/*==============================================================================
 * WIFI STREAM (don't change anything here)
 *============================================================================*/

#ifdef REMOTE_SERVER_IP
  WiFiClientStream stream(IPAddress(REMOTE_SERVER_IP), NETWORK_PORT);
#else
  WiFiServerStream stream(NETWORK_PORT);
#endif

/*==============================================================================
 * PIN IGNORE MACROS (don't change anything here)
 *============================================================================*/

#if defined(WIFI_101) && !defined(ARDUINO_SAMD_MKR1000)
// ignore SPI pins, pin 5 (reset WiFi101 shield), pin 7 (WiFi handshake) and pin 10 (WiFi SS)
// also don't ignore SS pin if it's not pin 10. Not needed for Arduino MKR1000.
#define IS_IGNORE_PIN(p)  ((p) == 10 || (IS_PIN_SPI(p) && (p) != SS) || (p) == 5 || (p) == 7)

#elif defined(ARDUINO_WIFI_SHIELD) && defined(__AVR_ATmega32U4__)
// ignore SPI pins, pin 4 (SS for SD-Card on WiFi-shield), pin 7 (WiFi handshake) and pin 10 (WiFi SS)
// On Leonardo, pin 24 maps to D4 and pin 28 maps to D10
#define IS_IGNORE_PIN(p)  ((IS_PIN_SPI(p) || (p) == 4) || (p) == 7 || (p) == 10 || (p) == 24 || (p) == 28)

#elif defined(ARDUINO_WIFI_SHIELD)
// ignore SPI pins, pin 4 (SS for SD-Card on WiFi-shield), pin 7 (WiFi handshake) and pin 10 (WiFi SS)
#define IS_IGNORE_PIN(p)  ((IS_PIN_SPI(p) || (p) == 4) || (p) == 7 || (p) == 10)

#elif defined(ESP8266_WIFI) && defined(SERIAL_DEBUG)
#define IS_IGNORE_PIN(p)  ((p) == 1)

#endif

/*==============================================================================
 * FIRMATA FEATURE CONFIGURATION
 *
 * Comment out the include and declaration for any features that you do not need
 * below.
 *
 * WARNING: Including all of the following features (especially if also using
 * Ethernet) may exceed the Flash and/or RAM of lower memory boards such as the
 * Arduino Uno or Leonardo.
 *============================================================================*/

#include <DigitalInputFirmata.h>
DigitalInputFirmata digitalInput;

#include <DigitalOutputFirmata.h>
DigitalOutputFirmata digitalOutput;

#include <AnalogInputFirmata.h>
AnalogInputFirmata analogInput;

#include <AnalogOutputFirmata.h>
AnalogOutputFirmata analogOutput;

#include <Servo.h>
#include <ServoFirmata.h>
ServoFirmata servo;
// ServoFirmata depends on AnalogOutputFirmata
#if defined ServoFirmata_h && ! defined AnalogOutputFirmata_h
#error AnalogOutputFirmata must be included to use ServoFirmata
#endif

#include <Wire.h>
#include <I2CFirmata.h>
I2CFirmata i2c;

#include <OneWireFirmata.h>
OneWireFirmata oneWire;

// StepperFirmata is deprecated as of ConfigurableFirmata v2.10.0. Please update your
// client implementation to use the new, more full featured and scalable AccelStepperFirmata.
#include <StepperFirmata.h>
StepperFirmata stepper;

#include <AccelStepperFirmata.h>
AccelStepperFirmata accelStepper;

#include <SerialFirmata.h>
SerialFirmata serial;

#include <FirmataExt.h>
FirmataExt firmataExt;

#include <FirmataScheduler.h>
FirmataScheduler scheduler;

// To add Encoder support you must first install the FirmataEncoder and Encoder libraries:
// https://github.com/firmata/FirmataEncoder
// https://www.pjrc.com/teensy/td_libs_Encoder.html
// #include <Encoder.h>
// #include <FirmataEncoder.h>
// FirmataEncoder encoder;

/*===================================================================================
 * END FEATURE CONFIGURATION - you should not need to change anything below this line
 *==================================================================================*/

// dependencies. Do not comment out the following lines
#if defined AnalogOutputFirmata_h || defined ServoFirmata_h
#include <AnalogWrite.h>
#endif

#if defined AnalogInputFirmata_h || defined I2CFirmata_h || defined FirmataEncoder_h
#include <FirmataReporting.h>
FirmataReporting reporting;
#endif


#ifdef STATIC_IP_ADDRESS
IPAddress local_ip(STATIC_IP_ADDRESS);
#endif
#ifdef SUBNET_MASK
IPAddress subnet(SUBNET_MASK);
#endif
#ifdef GATEWAY_IP_ADDRESS
IPAddress gateway(GATEWAY_IP_ADDRESS);
#endif

int connectionAttempts = 0;
bool streamConnected = false;

/*==============================================================================
 * FUNCTIONS
 *============================================================================*/

void systemResetCallback()
{
  // initialize a default state

  // pins with analog capability default to analog input
  // otherwise, pins default to digital output
  for (byte i = 0; i < TOTAL_PINS; i++) {
    if (IS_PIN_ANALOG(i)) {
#ifdef AnalogInputFirmata_h
      // turns off pull-up, configures everything
      Firmata.setPinMode(i, PIN_MODE_ANALOG);
#endif
    } else if (IS_PIN_DIGITAL(i)) {
#ifdef DigitalOutputFirmata_h
      // sets the output to 0, configures portConfigInputs
      Firmata.setPinMode(i, OUTPUT);
#endif
    }
  }

#ifdef FirmataExt_h
  firmataExt.reset();
#endif
}

/*
 * Called when a TCP connection is either connected or disconnected.
 * TODO:
 * - report connected or reconnected state to host (to be added to protocol)
 */
void hostConnectionCallback(byte state)
{
  switch (state) {
    case HOST_CONNECTION_CONNECTED:
      DEBUG_PRINTLN( "TCP connection established" );
      break;
    case HOST_CONNECTION_DISCONNECTED:
      DEBUG_PRINTLN( "TCP connection disconnected" );
      break;
  }
}

void printWifiStatus() {
  if ( WiFi.status() != WL_CONNECTED )
  {
    DEBUG_PRINT( "WiFi connection failed. Status value: " );
    DEBUG_PRINTLN( WiFi.status() );
  }
  else
  {
    // print the SSID of the network you're attached to:
    DEBUG_PRINT( "SSID: " );
    DEBUG_PRINTLN( WiFi.SSID() );

    // print your WiFi shield's IP address:
    DEBUG_PRINT( "IP Address: " );
    IPAddress ip = WiFi.localIP();
    DEBUG_PRINTLN( ip );

    // print the received signal strength:
    DEBUG_PRINT( "signal strength (RSSI): " );
    long rssi = WiFi.RSSI();
    DEBUG_PRINT( rssi );
    DEBUG_PRINTLN( " dBm" );
  }
}

/*
 * ConfigurableFirmataWiFi communicates with WiFi shields over SPI. Therefore all
 * SPI pins must be set to IGNORE. Otherwise Firmata would break SPI communication.
 * Additional pins may also need to be ignored depending on the particular board or
 * shield in use.
 */
void ignorePins()
{
#ifdef IS_IGNORE_PIN
  for (byte i = 0; i < TOTAL_PINS; i++) {
    if (IS_IGNORE_PIN(i)) {
      Firmata.setPinMode(i, PIN_MODE_IGNORE);
    }
  }
#endif

  //Set up controls for the Arduino WiFi Shield SS for the SD Card
#ifdef ARDUINO_WIFI_SHIELD
  // Arduino WiFi Shield has SD SS wired to D4
  pinMode(PIN_TO_DIGITAL(4), OUTPUT);    // switch off SD card bypassing Firmata
  digitalWrite(PIN_TO_DIGITAL(4), HIGH); // SS is active low;

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  pinMode(PIN_TO_DIGITAL(53), OUTPUT); // configure hardware SS as output on MEGA
#endif

#endif //ARDUINO_WIFI_SHIELD
}

void initTransport()
{
  // This statement will clarify how a connection is being made
  DEBUG_PRINT( "ConfigurableFirmataWiFi will attempt a WiFi connection " );
#if defined(WIFI_101)
  DEBUG_PRINTLN( "using the WiFi 101 library." );
#elif defined(ARDUINO_WIFI_SHIELD)
  DEBUG_PRINTLN( "using the legacy WiFi library." );
#elif defined(ESP8266_WIFI)
  DEBUG_PRINTLN( "using the ESP8266 WiFi library." );
  //else should never happen here as error-checking in wifiConfig.h will catch this
#endif  //defined(WIFI_101)

  // Configure WiFi IP Address
#ifdef STATIC_IP_ADDRESS
  DEBUG_PRINT( "Using static IP: " );
  DEBUG_PRINTLN( local_ip );
#if defined(ESP8266_WIFI) || (defined(SUBNET_MASK) && defined(GATEWAY_IP_ADDRESS))
  stream.config( local_ip , gateway, subnet );
#else
  // you can also provide a static IP in the begin() functions, but this simplifies
  // ifdef logic in this sketch due to support for all different encryption types.
  stream.config( local_ip );
#endif
#else
  DEBUG_PRINTLN( "IP will be requested from DHCP ..." );
#endif

  stream.attach(hostConnectionCallback);

  // Configure WiFi security and initiate WiFi connection
#if defined(WIFI_WEP_SECURITY)
  DEBUG_PRINT( "Attempting to connect to WEP SSID: " );
  DEBUG_PRINTLN(ssid);
  stream.begin(ssid, wep_index, wep_key);
#elif defined(WIFI_WPA_SECURITY)
  DEBUG_PRINT( "Attempting to connect to WPA SSID: " );
  DEBUG_PRINTLN(ssid);
  stream.begin(ssid, wpa_passphrase);
#else                          //OPEN network
  DEBUG_PRINTLN( "Attempting to connect to open SSID: " );
  DEBUG_PRINTLN(ssid);
  stream.begin(ssid);
#endif //defined(WIFI_WEP_SECURITY)
  DEBUG_PRINTLN( "WiFi setup done" );

  // Wait for connection to access point to be established.
  while (WiFi.status() != WL_CONNECTED && ++connectionAttempts <= MAX_CONN_ATTEMPTS) {
    delay(500);
    DEBUG_PRINT(".");
  }
  printWifiStatus();
}

void initFirmata()
{
  Firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);

#ifdef FirmataExt_h
#ifdef DigitalInputFirmata_h
  firmataExt.addFeature(digitalInput);
#endif
#ifdef DigitalOutputFirmata_h
  firmataExt.addFeature(digitalOutput);
#endif
#ifdef AnalogInputFirmata_h
  firmataExt.addFeature(analogInput);
#endif
#ifdef AnalogOutputFirmata_h
  firmataExt.addFeature(analogOutput);
#endif
#ifdef ServoFirmata_h
  firmataExt.addFeature(servo);
#endif
#ifdef I2CFirmata_h
  firmataExt.addFeature(i2c);
#endif
#ifdef OneWireFirmata_h
  firmataExt.addFeature(oneWire);
#endif
#ifdef StepperFirmata_h
  firmataExt.addFeature(stepper);
#endif
#ifdef AccelStepperFirmata_h
firmataExt.addFeature(accelStepper);
#endif
#ifdef SerialFirmata_h
  firmataExt.addFeature(serial);
#endif
#ifdef FirmataReporting_h
  firmataExt.addFeature(reporting);
#endif
#ifdef FirmataScheduler_h
  firmataExt.addFeature(scheduler);
#endif
#ifdef FirmataEncoder_h
  firmataExt.addFeature(encoder);
#endif
#endif
  /* systemResetCallback is declared here (in ConfigurableFirmata.ino) */
  Firmata.attach(SYSTEM_RESET, systemResetCallback);

  ignorePins();

  // Initialize Firmata to use the WiFi stream object as the transport.
  Firmata.begin(stream);
  Firmata.parse(SYSTEM_RESET);  // reset to default config
}


/*==============================================================================
 * SETUP()
 *============================================================================*/

void setup()
{
  DEBUG_BEGIN(9600);

  initTransport();

  initFirmata();
}

/*==============================================================================
 * LOOP()
 *============================================================================*/
void loop()
{
#ifdef DigitalInputFirmata_h
  /* DIGITALREAD - as fast as possible, check for changes and output them to the
   * stream buffer using Firmata.write()  */
  digitalInput.report();
#endif

  /* STREAMREAD - processing incoming message as soon as possible, while still
   * checking digital inputs.  */
  while (Firmata.available()) {
    Firmata.processInput();
#ifdef FirmataScheduler_h
    if (!Firmata.isParsingMessage()) {
      goto runtasks;
    }
  }
  if (!Firmata.isParsingMessage()) {
runtasks: scheduler.runTasks();
#endif
  }

  // TODO - ensure that Stream buffer doesn't go over 60 bytes

#ifdef FirmataReporting_h
  if (reporting.elapsed()) {
#ifdef AnalogInputFirmata_h
    /* ANALOGREAD - do all analogReads() at the configured sampling interval */
    analogInput.report();
#endif
#ifdef I2CFirmata_h
    // report i2c data for all device with read continuous mode enabled
    i2c.report();
#endif
#ifdef FirmataEncoder_h
    // report encoders positions if reporting enabled.
    encoder.report();
#endif
  }
#endif
#ifdef StepperFirmata_h
  stepper.update();
#endif
#ifdef AccelStepperFirmata_h
accelStepper.update();
#endif
#ifdef SerialFirmata_h
  serial.update();
#endif

  // keep the WiFi connection live. Attempts to reconnect automatically if disconnected.
  stream.maintain();
}
