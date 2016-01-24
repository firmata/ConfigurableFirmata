/*
  Copyright (C) 2006-2008 Hans-Christoph Steiner.  All rights reserved.
  Copyright (C) 2010-2011 Paul Stoffregen.  All rights reserved.
  Copyright (C) 2009 Shigeru Kobayashi.  All rights reserved.
  Copyright (C) 2013 Norbert Truchsess. All rights reserved.
  Copyright (C) 2009-2016 Jeff Hoefs.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

  Last updated by Jeff Hoefs: January 23rd, 2016
*/

#include <ConfigurableFirmata.h>
#include "StepperFirmata.h"
#include "utility/FirmataStepper.h"

boolean StepperFirmata::handlePinMode(byte pin, int mode)
{
  if (mode == PIN_MODE_STEPPER) {
    if (IS_PIN_DIGITAL(pin)) {
      digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable PWM
      pinMode(PIN_TO_DIGITAL(pin), OUTPUT);
      return true;
    }
  }
  return false;
}

void StepperFirmata::handleCapability(byte pin)
{
  if (IS_PIN_DIGITAL(pin)) {
    Firmata.write(PIN_MODE_STEPPER);
    Firmata.write(21); //21 bits used for number of steps
  }
}

/*==============================================================================
 * SYSEX-BASED commands
 *============================================================================*/

boolean StepperFirmata::handleSysex(byte command, byte argc, byte *argv)
{
  if (command == STEPPER_DATA) {
    byte stepCommand, deviceNum, directionPin, stepPin, stepDirection;
    byte interface, interfaceType;
    byte motorPin3, motorPin4;
    unsigned int stepsPerRev;
    long numSteps;
    int stepSpeed;
    int accel;
    int decel;

    stepCommand = argv[0];
    deviceNum = argv[1];

    if (deviceNum < MAX_STEPPERS) {
      if (stepCommand == STEPPER_CONFIG) {

        interface = argv[2]; // upper 4 bits are the stepDelay, lower 4 bits are the interface type
        interfaceType = interface & 0x0F; // the interface type is specified by the lower 4 bits
        stepsPerRev = (argv[3] + (argv[4] << 7));

        directionPin = argv[5]; // or motorPin1 for TWO_WIRE or FOUR_WIRE interface
        stepPin = argv[6]; // // or motorPin2 for TWO_WIRE or FOUR_WIRE interface
        if (Firmata.getPinMode(directionPin) == PIN_MODE_IGNORE || Firmata.getPinMode(stepPin) == PIN_MODE_IGNORE)
          return false;
        Firmata.setPinMode(directionPin, PIN_MODE_STEPPER);
        Firmata.setPinMode(stepPin, PIN_MODE_STEPPER);

        if (!stepper[deviceNum]) {
          numSteppers++;
        }
        if (interfaceType == FirmataStepper::DRIVER || interfaceType == FirmataStepper::TWO_WIRE) {
          stepper[deviceNum] = new FirmataStepper(interface, stepsPerRev, directionPin, stepPin);
        } else if (interfaceType == FirmataStepper::FOUR_WIRE) {
          motorPin3 = argv[7];
          motorPin4 = argv[8];
          if (Firmata.getPinMode(motorPin3) == PIN_MODE_IGNORE || Firmata.getPinMode(motorPin4) == PIN_MODE_IGNORE)
            return false;
          Firmata.setPinMode(motorPin3, PIN_MODE_STEPPER);
          Firmata.setPinMode(motorPin4, PIN_MODE_STEPPER);
          stepper[deviceNum] = new FirmataStepper(interface, stepsPerRev, directionPin, stepPin, motorPin3, motorPin4);
        }
      }
      else if (stepCommand == STEPPER_STEP) {
        stepDirection = argv[2];
        numSteps = (long)argv[3] | ((long)argv[4] << 7) | ((long)argv[5] << 14);
        stepSpeed = (argv[6] + (argv[7] << 7));

        if (stepDirection == 0) {
          numSteps *= -1;
        }
        if (stepper[deviceNum]) {
          if (argc >= 8 && argc < 12) {
            // num steps, speed (0.01*rad/sec)
            stepper[deviceNum]->setStepsToMove(numSteps, stepSpeed);
          } else if (argc == 12) {
            accel = (argv[8] + (argv[9] << 7));
            decel = (argv[10] + (argv[11] << 7));
            // num steps, speed (0.01*rad/sec), accel (0.01*rad/sec^2), decel (0.01*rad/sec^2)
            stepper[deviceNum]->setStepsToMove(numSteps, stepSpeed, accel, decel);
          }
        }
      }
      return true;
    }
  }
  return false;
}

/*==============================================================================
 * SETUP()
 *============================================================================*/

void StepperFirmata::reset()
{
  for (byte i = 0; i < MAX_STEPPERS; i++) {
    if (stepper[i]) {
      free(stepper[i]);
      stepper[i] = 0;
    }
  }
  numSteppers = 0;
}

/*==============================================================================
 * LOOP()
 *============================================================================*/
void StepperFirmata::update()
{
  if (numSteppers > 0) {
    // if one or more stepper motors are used, update their position
    for (byte i = 0; i < MAX_STEPPERS; i++) {
      if (stepper[i]) {
        bool done = stepper[i]->update();
        // send command to client application when stepping is complete
        if (done) {
          Firmata.write(START_SYSEX);
          Firmata.write(STEPPER_DATA);
          Firmata.write(i);
          Firmata.write(END_SYSEX);
        }
      }
    }
  }
}
