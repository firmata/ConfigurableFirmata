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

  Last updated September 16th, 2017
*/

/*
 README

 This is an example use of ConfigurableFirmata with BLE.

 ConfigurableFirmataBLE enables the use of Firmata over a BLE connection. To configure your
 connection, follow the instructions in the BLE CONFIGURATION section of this file.

 To use ConfigurableFirmataBLE you will need to have one of the following boards or shields:
 - Arduino 101 (recommended)
 - RedBear BLE Nano ** works with modifications **
 - RedBear BLE Shield v2 ** to be verfied **

 If you are using an Arduino 101, make sure you have the Intel Curie Boards package v1.0.6
 or higer installed via the Arduino Boards Manager.
*/

#include "ConfigurableFirmata.h"

// min cannot be < 0x0006. Adjust max if necessary
#define FIRMATA_BLE_MIN_INTERVAL    0x0006 // 7.5ms (7.5 / 1.25)
#define FIRMATA_BLE_MAX_INTERVAL    0x0018 // 30ms (30 / 1.25)

/*==============================================================================
 * BLE CONFIGURATION
 *
 * If you are using an Arduino 101, you do not need to make any changes to this
 * section of the file unless you need a unique ble local name. If you are using
 * another supported BLE board or shield, follow the instructions for the specific
 * board or shield below.
 *============================================================================*/

// change this to a unique name per board if running StandardFirmataBLE on multiple boards
// within the same physical space
#define FIRMATA_BLE_LOCAL_NAME "FIRMATA"


/*
 * Arduino 101
 *
 * Make sure you have the Intel Curie Boards package v1.0.6 or higher installed via the Arduino
 * Boards Manager.
 *
 * Test script: https://gist.github.com/soundanalogous/927360b797574ed50e27
 */
#ifdef _VARIANT_ARDUINO_101_X_
#include <CurieBLE.h>
#include "utility/BLEStream.h"
BLEStream stream;
#endif


/*
 * RedBearLab BLE Nano (with default switch settings)
 *
 * Blocked on this issue: https://github.com/RedBearLab/nRF51822-Arduino/pull/97
 * Works with modifications. See comments at top of the test script referenced below.
 * When the RBL nRF51822-Arduino library issue is resolved, this should work witout
 * any modifications.
 *
 * Test script: https://gist.github.com/soundanalogous/d39bb3eb36333a0906df
 *
 * Note: If you have changed the solder jumpers on the Nano you may encounter issues since
 * the pins are currently mapped in Firmata only for the default (factory) jumper settings.
 */
#ifdef BLE_NANO
#include <BLEPeripheral.h>
#include "utility/BLEStream.h"
BLEStream stream;
#endif

/*
 * RedBearLab BLE Shield
 *
 * If you are using a RedBearLab BLE shield, uncomment the define below.
 * Also, change the define for BLE_RST if you have the jumper set to pin 7 rather than pin 4.
 *
 * You will need to use the shield with an Arduino Zero, Due, Mega, or other board with sufficient
 * Flash and RAM. Arduino Uno, Leonardo and other ATmega328p and Atmega32u4 boards do not have
 * enough memory to run StandardFirmataBLE.
 *
 * TODO: verify if this works and with which boards it works.
 *
 * Test script: https://gist.github.com/soundanalogous/927360b797574ed50e27
 */
//#define REDBEAR_BLE_SHIELD

#ifdef REDBEAR_BLE_SHIELD
#include <SPI.h>
#include <BLEPeripheral.h>
#include "utility/BLEStream.h"

#define BLE_REQ  9
#define BLE_RDY  8
#define BLE_RST  4 // 4 or 7 via jumper on shield

BLEStream stream(BLE_REQ, BLE_RDY, BLE_RST);
#endif


#if defined(BLE_REQ) && defined(BLE_RDY) && defined(BLE_RST)
#define IS_IGNORE_BLE_PINS(p) ((p) == BLE_REQ || (p) == BLE_RDY || (p) == BLE_RST)
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

void ignorePins()
{
#ifdef BLE_REQ
  for (byte i = 0; i < TOTAL_PINS; i++) {
    if (IS_IGNORE_BLE_PINS(i)) {
      Firmata.setPinMode(i, PIN_MODE_IGNORE);
    }
  }
#endif
}

void initTransport()
{
  stream.setLocalName(FIRMATA_BLE_LOCAL_NAME);
  // set the BLE connection interval - this is the fastest interval you can read inputs
  stream.setConnectionInterval(FIRMATA_BLE_MIN_INTERVAL, FIRMATA_BLE_MAX_INTERVAL);
  // set how often the BLE TX buffer is flushed (if not full)
  stream.setFlushInterval(FIRMATA_BLE_MAX_INTERVAL);

  stream.begin();
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

  // Initialize Firmata to use the BLE stream object as the transport.
  Firmata.begin(stream);
  Firmata.parse(SYSTEM_RESET);  // reset to default config
}

void setup()
{
  initTransport();

  initFirmata();
}

/*==============================================================================
 * LOOP()
 *============================================================================*/
void loop()
{
  // do not process data if no BLE connection is established
  // poll will send the TX buffer at the specified flush interval or when the buffer is full
  if (!stream.poll()) return;

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

}
