/*
  Copyright (C) 2006-2008 Hans-Christoph Steiner.  All rights reserved.
  Copyright (C) 2010-2011 Paul Stoffregen.  All rights reserved.
  Copyright (C) 2009 Shigeru Kobayashi.  All rights reserved.
  Copyright (C) 2013 Norbert Truchsess. All rights reserved.
  Copyright (C) 2009-2017 Jeff Hoefs.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

  Last updated: September 16th, 2017
*/

#include <ConfigurableFirmata.h>
#include "AccelStepperFirmata.h"
#include "utility/AccelStepper.h"
#include "utility/MultiStepper.h"

boolean AccelStepperFirmata::handlePinMode(byte pin, int mode)
{
  if (mode == PIN_MODE_STEPPER) {
    if (IS_PIN_DIGITAL(pin)) {
      pinMode(PIN_TO_DIGITAL(pin), OUTPUT);
      return true;
    }
  }
  return false;
}

void AccelStepperFirmata::handleCapability(byte pin)
{
  if (IS_PIN_DIGITAL(pin)) {
    Firmata.write(PIN_MODE_STEPPER);
    Firmata.write(21); //21 bits used for number of steps
  }
}

// Send position data when it's requested or a move completes
void AccelStepperFirmata::reportPosition(byte deviceNum, bool complete)
{
  if (stepper[deviceNum]) {
    byte data[5];
    long position = stepper[deviceNum]->currentPosition();
    encode32BitSignedInteger(position, data);

    Firmata.write(START_SYSEX);
    Firmata.write(ACCELSTEPPER_DATA);
    if (complete) {
      Firmata.write(ACCELSTEPPER_MOVE_COMPLETE);
    } else {
      Firmata.write(ACCELSTEPPER_REPORT_POSITION);
    }
    Firmata.write(deviceNum);
    Firmata.write(data[0]);
    Firmata.write(data[1]);
    Firmata.write(data[2]);
    Firmata.write(data[3]);
    Firmata.write(data[4]);
    Firmata.write(END_SYSEX);
  }
}

void AccelStepperFirmata::reportGroupComplete(byte deviceNum)
{
  if (group[deviceNum]) {
    Firmata.write(START_SYSEX);
    Firmata.write(ACCELSTEPPER_DATA);
    Firmata.write(MULTISTEPPER_MOVE_COMPLETE);
    Firmata.write(deviceNum);
    Firmata.write(END_SYSEX);
  }
}

/*==============================================================================
 * SYSEX-BASED commands
 *============================================================================*/

boolean AccelStepperFirmata::handleSysex(byte command, byte argc, byte *argv)
{
  if (command == ACCELSTEPPER_DATA) {
    byte stepCommand, deviceNum, interface, wireCount, stepType;
    byte stepOrMotorPin1, directionOrMotorPin2;
    byte motorPin3 = 0, motorPin4 = 0, enablePin = 0, invertPins = 0;
    long numSteps;

    unsigned int index = 0;

    stepCommand = argv[index++];
    deviceNum = argv[index++];

    if (deviceNum < MAX_ACCELSTEPPERS) {

      if (stepCommand == ACCELSTEPPER_CONFIG) {
        interface = argv[index++];
        wireCount = (interface & 0x70) >> 4; // upper 3 bits are the wire count
        stepType = (interface & 0x0e) >> 1; // next 3 bits are the step type
        stepOrMotorPin1 = argv[index++]; // Step pin for driver or MotorPin1
        directionOrMotorPin2 = argv[index++]; // Direction pin for driver or motorPin2

        if (Firmata.getPinMode(directionOrMotorPin2) == PIN_MODE_IGNORE
            || Firmata.getPinMode(stepOrMotorPin1) == PIN_MODE_IGNORE) {
          return false;
        }

        Firmata.setPinMode(stepOrMotorPin1, PIN_MODE_STEPPER);
        Firmata.setPinMode(directionOrMotorPin2, PIN_MODE_STEPPER);

        if (!stepper[deviceNum]) {
          numSteppers++;
        }

        if (wireCount >= 3) {
          motorPin3 = argv[index++];
          if (Firmata.getPinMode(motorPin3) == PIN_MODE_IGNORE)
            return false;
          Firmata.setPinMode(motorPin3, PIN_MODE_STEPPER);
        }

        if (wireCount >= 4) {
          motorPin4 = argv[index++];
          if (Firmata.getPinMode(motorPin4) == PIN_MODE_IGNORE)
            return false;
          Firmata.setPinMode(motorPin4, PIN_MODE_STEPPER);
        }

        // If we have an enable pin
        if (interface & 0x01) {
          enablePin = argv[index++];
          if (Firmata.getPinMode(enablePin) == PIN_MODE_IGNORE)
            return false;
        }

        // Instantiate our stepper
        if (wireCount == 1) {
          stepper[deviceNum] = new AccelStepper(AccelStepper::DRIVER, stepOrMotorPin1, directionOrMotorPin2);
        } else if (wireCount == 2) {
          stepper[deviceNum] = new AccelStepper(AccelStepper::FULL2WIRE, stepOrMotorPin1, directionOrMotorPin2);
        } else if (wireCount == 3 && stepType == STEP_TYPE_WHOLE) {
          stepper[deviceNum] = new AccelStepper(AccelStepper::FULL3WIRE, stepOrMotorPin1, directionOrMotorPin2, motorPin3);
        } else if (wireCount == 3 && stepType == STEP_TYPE_HALF) {
          stepper[deviceNum] = new AccelStepper(AccelStepper::HALF3WIRE, stepOrMotorPin1, directionOrMotorPin2, motorPin3);
        } else if (wireCount == 4 && stepType == STEP_TYPE_WHOLE) {
          stepper[deviceNum] = new AccelStepper(AccelStepper::FULL4WIRE, stepOrMotorPin1, directionOrMotorPin2, motorPin3, motorPin4, false);
        } else if (wireCount == 4 && stepType == STEP_TYPE_HALF) {
          stepper[deviceNum] = new AccelStepper(AccelStepper::HALF4WIRE, stepOrMotorPin1, directionOrMotorPin2, motorPin3, motorPin4, false);
        }

        // If there is still another byte to read we must be inverting some pins
        if (argc >= index) {
          invertPins = argv[index];
          if (wireCount == 1) {
            stepper[deviceNum]->setPinsInverted(invertPins & 0x01, invertPins >> 1 & 0x01, invertPins >> 4 & 0x01);
          } else {
            stepper[deviceNum]->setPinsInverted(invertPins & 0x01, invertPins >> 1 & 0x01, invertPins >> 2 & 0x01, invertPins >> 3 & 0x01, invertPins >> 4 & 0x01);
          }
        }

        if (interface & 0x01) {
          stepper[deviceNum]->setEnablePin(enablePin);
        }

        /*
          Default to no acceleration. We set the acceleration value high enough that our speed is
          reached on the first step of a movement.
          More info about this hack in ACCELSTEPPER_SET_ACCELERATION.

          The lines where we are setting the speed twice are necessary because if the max speed doesn't change
          from the default value then our time to next step does not get computed after raising the acceleration.
        */
        stepper[deviceNum]->setMaxSpeed(2.0);
        stepper[deviceNum]->setMaxSpeed(1.0);
        stepper[deviceNum]->setAcceleration(MAX_ACCELERATION);

        isRunning[deviceNum] = false;

      }

      else if (stepCommand == ACCELSTEPPER_STEP) {
        numSteps = decode32BitSignedInteger(argv[2], argv[3], argv[4], argv[5], argv[6]);

        if (stepper[deviceNum]) {
          stepper[deviceNum]->move(numSteps);
          isRunning[deviceNum] = true;
        }

      }

      else if (stepCommand == ACCELSTEPPER_ZERO) {
        if (stepper[deviceNum]) {
          stepper[deviceNum]->setCurrentPosition(0);
        }
      }

      else if (stepCommand == ACCELSTEPPER_TO) {
        if (stepper[deviceNum]) {
          numSteps = decode32BitSignedInteger(argv[2], argv[3], argv[4], argv[5], argv[6]);
          stepper[deviceNum]->moveTo(numSteps);
          isRunning[deviceNum] = true;
        }
      }

      else if (stepCommand == ACCELSTEPPER_ENABLE) {
        if (stepper[deviceNum]) {
          if (argv[2] == 0x00) {
            stepper[deviceNum]->disableOutputs();
          } else {
            stepper[deviceNum]->enableOutputs();
          }
        }
      }

      else if (stepCommand == ACCELSTEPPER_STOP) {
        if (stepper[deviceNum]) {
          stepper[deviceNum]->stop();
          isRunning[deviceNum] = false;
          reportPosition(deviceNum, true);
        }
      }

      else if (stepCommand == ACCELSTEPPER_REPORT_POSITION) {
        if (stepper[deviceNum]) {
          reportPosition(deviceNum, false);
        }
      }

      else if (stepCommand == ACCELSTEPPER_SET_ACCELERATION) {
        float decodedAcceleration = decodeCustomFloat(argv[2], argv[3], argv[4], argv[5]);

        if (stepper[deviceNum]) {
          /*
            <HACK>
            All firmata instances of accelStepper have an acceleration value. If a user does not
            want acceleration we just set the acceleration value high enough so
            that the chosen speed will be realized by the first step of a movement.
            This simplifies some of the logic in StepperFirmata and gives us more flexibility
            should an alternative stepper library become available at a future date.
          */
          if (decodedAcceleration == 0.0) {
            stepper[deviceNum]->setAcceleration(MAX_ACCELERATION);
          } else {
            stepper[deviceNum]->setAcceleration(decodedAcceleration);
          }
          /*
            </HACK>
          */
        }

      }

      else if (stepCommand == ACCELSTEPPER_SET_SPEED) {
        // Sets the maxSpeed for accelStepper. We do not use setSpeed here because
        // all instances of accelStepper that have been created by firmata are
        // using acceleration. More info about this hack in ACCELSTEPPER_SET_ACCELERATION.
        float speed = decodeCustomFloat(argv[2], argv[3], argv[4], argv[5]);

        if (stepper[deviceNum]) {
          stepper[deviceNum]->setMaxSpeed(speed);
        }
      }

      else if (stepCommand == MULTISTEPPER_CONFIG) {
        if (!group[deviceNum]) {
          numGroups++;
          group[deviceNum] = new MultiStepper();
        }

        for (byte i = index; i < argc; i++) {
          byte stepperNumber = argv[i];

          if (stepper[stepperNumber]) {
            groupStepperCount[deviceNum]++;
            group[deviceNum]->addStepper(*stepper[stepperNumber]);
          }
        }

        groupIsRunning[deviceNum] = false;
      }

      else if (stepCommand == MULTISTEPPER_TO) {
        groupIsRunning[deviceNum] = true;
        long positions[groupStepperCount[deviceNum]];

        for (byte i = 0, offset = 0; i < groupStepperCount[deviceNum]; i++) {
          offset = index + (i * 5);
          positions[i] = decode32BitSignedInteger(argv[offset], argv[offset + 1], argv[offset + 2], argv[offset + 3], argv[offset + 4]);
        }

        group[deviceNum]->moveTo(positions);
      }

      else if (stepCommand == MULTISTEPPER_STOP) {
        groupIsRunning[deviceNum] = false;
        reportGroupComplete(deviceNum);
      }
    }
    return true;

  }
  return false;
}

/*==============================================================================
 * SETUP()
 *============================================================================*/

void AccelStepperFirmata::reset()
{
  for (byte i = 0; i < MAX_ACCELSTEPPERS; i++) {
    if (stepper[i]) {
      free(stepper[i]);
      stepper[i] = 0;
    }
  }
  numSteppers = 0;

  for (byte i = 0; i < MAX_GROUPS; i++) {
    if (group[i]) {
      free(group[i]);
      group[i] = 0;
    }
  }
  numGroups = 0;
}

/*==============================================================================
 * Helpers
 *============================================================================*/

float AccelStepperFirmata::decodeCustomFloat(byte arg1, byte arg2, byte arg3, byte arg4)
{
  long l4 = (long)arg4;
  long significand = (long)arg1 | (long)arg2 << 7 | (long)arg3 << 14 | (l4 & 0x03) << 21;
  float exponent = (float)(((l4 >> 2) & 0x0f) - 11);
  bool sign = (bool)((l4 >> 6) & 0x01);
  float result = (float)significand;

  if (sign) {
    result *= -1;
  }

  result = result * powf(10.0, exponent);

  return result;
}

long AccelStepperFirmata::decode32BitSignedInteger(byte arg1, byte arg2, byte arg3, byte arg4, byte arg5)
{
  long result = (long)arg1 | (long)arg2 << 7 | (long)arg3 << 14 | (long)arg4 << 21 | (((long)arg5 << 28) & 0x07);

  if ((long)arg5 >> 3 == 0x01) {
    result = result * -1;
  }

  return result;
}

void AccelStepperFirmata::encode32BitSignedInteger(long value, byte pdata[])
{
  bool inv = false;

  if (value < 0) {
    inv = true;
    value = value * -1;
  }

  pdata[0] = value & 0x7f;
  pdata[1] = (value >> 7) & 0x7f;
  pdata[2] = (value >> 14) & 0x7f;
  pdata[3] = (value >> 21) & 0x7f;
  pdata[4] = (value >> 28) & 0x7f;

  if (inv == true) {
    pdata[4] = pdata[4] | 0x08;
  }
}

/*==============================================================================
 * LOOP()
 *============================================================================*/
void AccelStepperFirmata::update()
{
  bool stepsLeft;

  if (numGroups > 0) {
    // if one or more groups exist,  update their position
    for (byte i = 0; i < MAX_GROUPS; i++) {
      if (group[i] && groupIsRunning[i] == true) {
        stepsLeft = group[i]->run();

        // send command to client application when stepping is complete
        if (stepsLeft != true) {
          groupIsRunning[i] = false;
          reportGroupComplete(i);
        }

      }
    }
  }

  if (numSteppers > 0) {
    // if one or more stepper motors are used, update their position
    for (byte i = 0; i < MAX_ACCELSTEPPERS; i++) {
      if (stepper[i] && isRunning[i] == true) {
        stepsLeft = stepper[i]->run();

        // send command to client application when stepping is complete
        if (!stepsLeft) {
          isRunning[i] = false;
          reportPosition(i, true);
        }

      }
    }
  }

}
