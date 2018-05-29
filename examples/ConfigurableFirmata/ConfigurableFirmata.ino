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
  Copyright (C) 2009-2017 Jeff Hoefs.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

  Last updated: September 16th, 2017
*/

/*
 README

 This is an example use of ConfigurableFirmata. The easiest way to create a configuration is to
 use http://firmatabuilder.com and select the communication transport and the firmata features
 to include and an Arduino sketch (.ino) file will be generated and downloaded automatically.

 To manually configure a sketch, copy this file and follow the instructions in the
 ETHERNET CONFIGURATION OPTION (if you want to use Ethernet instead of Serial/USB) and
 FIRMATA FEATURE CONFIGURATION sections in this file.
*/

#include "ConfigurableFirmata.h"

/*==============================================================================
 * ETHERNET CONFIGURATION OPTION
 *
 * By default Firmata uses the Serial-port (over USB) of the Arduino. ConfigurableFirmata may also
 * comunicate over ethernet using tcp/ip. To configure this sketch to use Ethernet instead of
 * Serial, uncomment the approprate includes for your particular hardware. See STEPS 1 - 5 below.
 * If you want to use Serial (over USB) then skip ahead to the FIRMATA FEATURE CONFIGURATION
 * section further down in this file.
 *
 * If you enable Ethernet, you will need a Firmata client library with a network transport that can
 * act as a server in order to establish a connection between ConfigurableFirmataEthernet and the
 * Firmata host application (your application).
 *
 * To use ConfigurableFirmata with Ethernet you will need to have one of the following
 * boards or shields:
 *
 *  - Arduino Ethernet shield (or clone)
 *  - Arduino Ethernet board (or clone)
 *  - Arduino Yun
 *
 *  If you are using an Arduino Ethernet shield you cannot use the following pins on
 *  the following boards. Firmata will ignore any requests to use these pins:
 *
 *  - Arduino Uno or other ATMega328 boards: (D4, D10, D11, D12, D13)
 *  - Arduino Mega: (D4, D10, D50, D51, D52, D53)
 *  - Arduino Leonardo: (D4, D10)
 *  - Arduino Due: (D4, D10)
 *  - Arduino Zero: (D4, D10)
 *
 *  If you are using an ArduinoEthernet board, the following pins cannot be used (same as Uno):
 *  - D4, D10, D11, D12, D13
 *============================================================================*/

// STEP 1 [REQUIRED]
// Uncomment / comment the appropriate set of includes for your hardware (OPTION A, B or C)

/*
 * OPTION A: Configure for Arduino Ethernet board or Arduino Ethernet shield (or clone)
 *
 * To configure ConfigurableFirmata to use the an Arduino Ethernet Shield or Arduino Ethernet
 * Board (both use the same WIZ5100-based Ethernet controller), uncomment the SPI and Ethernet
 * includes below.
 */
//#include <SPI.h>
//#include <Ethernet.h>


/*
 * OPTION B: Configure for a board or shield using an ENC28J60-based Ethernet controller,
 * uncomment out the UIPEthernet include below.
 *
 * The UIPEthernet-library can be downloaded
 * from: https://github.com/ntruchsess/arduino_uip
 */
//#include <UIPEthernet.h>


/*
 * OPTION C: Configure for Arduino Yun
 *
 * The Ethernet port on the Arduino Yun board can be used with Firmata in this configuration.
 * To execute StandardFirmataEthernet on Yun uncomment the Bridge and YunClient includes below.
 *
 * NOTE: in order to compile for the Yun you will also need to comment out some of the includes
 * and declarations in the FIRMATA FEATURE CONFIGURATION section later in this file. Including all
 * features exceeds the RAM and Flash memory of the Yun. Comment out anything you don't need.
 *
 * On Yun there's no need to configure local_ip and mac address as this is automatically
 * configured on the linux-side of Yun.
 *
 * Establishing a connection with the Yun may take several seconds.
 */
//#include <Bridge.h>
//#include <YunClient.h>

#if defined ethernet_h || defined UIPETHERNET_H || defined _YUN_CLIENT_H_
#define NETWORK_FIRMATA

// STEP 2 [REQUIRED for all boards and shields]
// replace with IP of the server you want to connect to, comment out if using 'remote_host'
#define remote_ip IPAddress(192, 168, 0, 1)
// OR replace with hostname of server you want to connect to, comment out if using 'remote_ip'
// #define remote_host "server.local"

// STEP 3 [REQUIRED unless using Arduino Yun]
// Replace with the port that your server is listening on
#define remote_port 3030

// STEP 4 [REQUIRED unless using Arduino Yun OR if not using DHCP]
// Replace with your board or Ethernet shield's IP address
// Comment out if you want to use DHCP
#define local_ip IPAddress(192, 168, 0, 6)

// STEP 5 [REQUIRED unless using Arduino Yun]
// replace with Ethernet shield mac. Must be unique for your network
const byte mac[] = {0x90, 0xA2, 0xDA, 0x0D, 0x07, 0x02};
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

// dependencies for Network Firmata. Do not comment out.
#ifdef NETWORK_FIRMATA
#if defined remote_ip && defined remote_host
#error "cannot define both remote_ip and remote_host at the same time!"
#endif
#include <EthernetClientStream.h>
#ifdef _YUN_CLIENT_H_
YunClient client;
#else
EthernetClient client;
#endif
#if defined remote_ip && !defined remote_host
#ifdef local_ip
EthernetClientStream stream(client, local_ip, remote_ip, NULL, remote_port);
#else
EthernetClientStream stream(client, IPAddress(0, 0, 0, 0), remote_ip, NULL, remote_port);
#endif
#endif
#if !defined remote_ip && defined remote_host
#ifdef local_ip
EthernetClientStream stream(client, local_ip, IPAddress(0, 0, 0, 0), remote_host, remote_port);
#else
EthernetClientStream stream(client, IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), remote_host, remote_port);
#endif
#endif
#endif

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

/*==============================================================================
 * SETUP()
 *============================================================================*/

void setup()
{
  /*
   * ETHERNET SETUP
   */
#ifdef NETWORK_FIRMATA
#ifdef _YUN_CLIENT_H_
  Bridge.begin();
#else
#ifdef local_ip
  Ethernet.begin((uint8_t *)mac, local_ip); //start Ethernet
#else
  Ethernet.begin((uint8_t *)mac); //start Ethernet using dhcp
#endif
#endif
  delay(1000);
#endif

  /*
   * FIRMATA SETUP
   */
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

  // Network Firmata communicates with Ethernet-shields over SPI. Therefor all
  // SPI-pins must be set to PIN_MODE_IGNORE. Otherwise Firmata would break SPI-communication.
  // add Pin 10 and configure pin 53 as output if using a MEGA with Ethernetshield.
  // No need to ignore pin 10 on MEGA with ENC28J60, as here pin 53 should be connected to SS:
#ifdef NETWORK_FIRMATA

  #ifndef _YUN_CLIENT_H_
  // ignore SPI and pin 4 that is SS for SD-Card on Ethernet-shield
  for (byte i = 0; i < TOTAL_PINS; i++) {
    if (IS_PIN_SPI(i)
        || 4 == i  // SD Card on Ethernet shield uses pin 4 for SS
        || 10 == i // Ethernet-shield uses pin 10 for SS
       ) {
      Firmata.setPinMode(i, PIN_MODE_IGNORE);
    }
  }
  //  pinMode(PIN_TO_DIGITAL(53), OUTPUT); configure hardware-SS as output on MEGA
  pinMode(PIN_TO_DIGITAL(4), OUTPUT); // switch off SD-card bypassing Firmata
  digitalWrite(PIN_TO_DIGITAL(4), HIGH); // SS is active low;
  #endif

  #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    pinMode(PIN_TO_DIGITAL(53), OUTPUT); // configure hardware SS as output on MEGA
  #endif

  // start up Network Firmata:
  Firmata.begin(stream);
#else
  // Uncomment to save a couple of seconds by disabling the startup blink sequence.
  // Firmata.disableBlinkVersion();

  // start up the default Firmata using Serial interface:
  Firmata.begin(57600);
#endif
  Firmata.parse(SYSTEM_RESET);  // reset to default config
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

  /* SEND STREAM WRITE BUFFER - TO DO: make sure that the stream buffer doesn't go over
   * 60 bytes. use a timer to sending an event character every 4 ms to
   * trigger the buffer to dump. */

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

#if defined NETWORK_FIRMATA && !defined local_ip &&!defined _YUN_CLIENT_H_
  // only necessary when using DHCP, ensures local IP is updated appropriately if it changes
  if (Ethernet.maintain()) {
    stream.maintain(Ethernet.localIP());
  }
#endif
}
