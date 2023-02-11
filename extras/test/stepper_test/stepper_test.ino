/*
 * To run this test suite, you must first install the ArduinoUnit library
 * to your Arduino/libraries/ directory.
 * You can get ArduinoUnit here: https://github.com/mmurdoch/arduinounit
 * Download version 2.0 or greater.
 */

#include <ArduinoUnit.h>
#include <AccelStepperFirmata.h>

void setup()
{
  Serial.begin(9600);
}

test(decode_custom_float)
{
  AccelStepperFirmata stepper;
  float result;

  result = stepper.decodeCustomFloat( 110, 92, 44, 32 );
  result = fabs(732.782 - result);

  assertLessOrEqual(result,.01);

}

test(decode_negative_custom_float)
{
  AccelStepperFirmata stepper;
  float result;

  result = stepper.decodeCustomFloat( 110, 92, 44, 96 );
  result = fabs(-732.782 - result);

  assertLessOrEqual(result,.01);

}

test(encode_32_bit_signed_integer)
{
  AccelStepperFirmata stepper;
  byte result[5];

  stepper.encode32BitSignedInteger(5786, result);

  assertEqual(result[0], 26);
  assertEqual(result[1], 45);
  assertEqual(result[2], 0);
  assertEqual(result[3], 0);
  assertEqual(result[4], 0);
}

test(encode_negative_32_bit_signed_integer)
{
  AccelStepperFirmata stepper;
  byte result[5];

  stepper.encode32BitSignedInteger(-5786, result);

  assertEqual(result[0], 26);
  assertEqual(result[1], 45);
  assertEqual(result[2], 0);
  assertEqual(result[3], 0);
  assertEqual(result[4], 8);
}

test(decode_32_bit_signed_integer)
{
  AccelStepperFirmata stepper;
  long result;

  result = stepper.decode32BitSignedInteger(26, 45, 0, 0, 0);

  assertEqual(result, 5786);
}

test(decode_negative_32_bit_signed_integer)
{
  AccelStepperFirmata stepper;
  long result;

  result = stepper.decode32BitSignedInteger(26, 45, 0, 0, 8);

  assertEqual(result, -5786);
}

void loop()
{
  Test::run();
}
