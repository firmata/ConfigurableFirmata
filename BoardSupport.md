# Board support information

This file gives additional information about the support for specific board types.

## AVR Based Boards (Arduino Uno, Arduino Nano and clones)

These are the most basic but also most used Arduino boards. Since their memory is limited, not all possible pin modes are supported at once. 
However, most of the time, not everything is required for the same project at once. Some of the older boards (i.e first Generation Duemilanove with ATmega168) are even further limited, as they have only 16k of Flash and 1k of RAM. 

The values given below are for the Arduino Uno, the most used board as of 2021.
| Property  | Value |
| ----------- | ------------------------------------ |
| Number of Pins  | 20 |
| Number of analog inputs | 6 |
| Flash Memory | 32kb |
| RAM | 2kb |
| PWM capable pins | Pins 3, 5, 6, 9, 10, 11 |
| Built-in LED | Yes, pin 13 |
| I2C, Bus 0 | Pin 18 SDA, Pin 19 SCL |
| SPI, Bus 0 | Pin 10 CS, Pin 11 MISO, Pin 12 MOSI, Pin 13 SCK |

## Arduino Due

This board is not just a new revision of the Arduino Uno. It has a 32 Bit microcontroller and significantly more memory than the AVR based boards. This has enough pins for even the largest projects. The Arduino Due is only supported with ConfigurableFirmata 2.11 or later.

Official pinout diagram is here: https://content.arduino.cc/assets/Pinout-Due_latest.pdf
| Property  | Value |
| ----------- | ------------------------------------ |
| Number of Pins  | 77, including some that are not accessible. These will report as having no features enabled. |
| Number of analog inputs | 12 |
| Flash Memory | 512kb |
| RAM | 96kb |
| PWM capable pins | Pins 2-10 (The DAC pins are not currently supported by ConfigurableFirmata) |
| Built-in LED | Yes, pin 13 |
| I2C, Bus 0 | Pin 20 SDA, Pin 21 SCL |
| I2C, Bus 1 | Not currently supported by ConfigurableFirmata |
| SPI, Bus 0 | Pin 77 CS (This pin is not accessible), Pin 75 MOSI, Pin 74 MISO, Pin 76 SCK (the latter three are on the SPI header, just next to the microcontroller). For CS, you have to use a GPIO pin |

## ESP32

The ESP32 (available in different variants) is one of the most powerful microcontrollers available. Development boards are significantly cheaper than all types of Arduino boards, so it's the most price-effective choice currently available, if you do not need the large number of pins of an Arduino Due or an Arduino Mega. 

Pinout diagrams differ by vendor. The ESP32 module is almost always sold with a carrier board, because the raw module does not come with pins and is very small. 

| Property  | Value |
| ----------- | ------------------------------------ |
| Number of Pins  | 40 total, 26 typically available for user programs
| Number of analog inputs | 15 |
| Flash Memory | 4MB default, variants with more are available |
| RAM | 320kb or more|
| PWM capable pins | All, 16 Pins can use PWM at once |
| Built-in LED | Usually pin 2, but not always present |
| I2C, Bus 0 | Pins 21 and 22|
| SPI, Bus 0 | Pins 18, 19 and 23 |

## [RP2040](https://www.raspberrypi.com/products/rp2040/) / Raspberry Pi [Pico, PicoW](https://www.raspberrypi.com/products/raspberry-pi-pico/), [RP2350](https://www.raspberrypi.com/products/rp2350/) / [Pico2 + Pico2W](https://www.raspberrypi.com/products/raspberry-pi-pico-2/) and compatible boards

First of all add support for RP2040 boards in the Arduino IDE 2, by going to Boards Manager within the Tools menu.
Enter "RP2040" in the search box, 
 - Select "Rasberryi Pi Pico/RP2040/RP2350 by Earle F. Philhower" (See 110+ [supported boards here...](https://github.com/earlephilhower/arduino-pico?tab=readme-ov-file#supported-boards) ) 
    **More documentation** about EarlyPh. library + VSCode + PlatformIO: [here...](https://arduino-pico.readthedocs.io/en/latest/)
 - You can also select old "Arduino Mbed OS RP2040 Boards" and install the latest version (4.2.4 at the time of writing 2025-04-16), but it will only support RP2040 = Pico1(W), and no temperature reading. Also it compiles slower, and many advanced features are not available. So it's not recommended any more.

> **! Note:** that version < 3.4 has a [bug in SERVO part](https://github.com/firmata/ConfigurableFirmata/pull/184), so it had to be disabled. You may manually download fixed code if you need SERVO, until v3.4+ is released.
> To disable it, put "//" before the line: `// #define ENABLE_SERVO`

### The RP2040 is supported by ConfigurableFirmata v2.11 or later. 

| Property  | Value (Logical pin numbers) |
| ----------- | ------------------------------------ |
| Number of Pins  | 30+1* |
| Number of analog inputs | 4+1* |
| *Built in [temperature sensor](https://github.com/earlephilhower/arduino-pico/discussions/2897#discussioncomment-12816194) | A4 |
| Flash Memory | 2Mb |
| RAM | 264kb |
| PWM capable pins | 16 |
| Built-in LED | Yes, pin 25 |
| I2C, Bus 0 | Pin 6 SDA, Pin 7 SCL |
| I2C, Bus 1 | Not currently supported by ConfigurableFirmata |
| SPI, Bus 0 | Pin 16 MISO, Pin 17 CS, Pin 18 SCK, Pin 19 MOSI |
| SPI, Bus 1 | Not currently supported by ConfigurableFirmata |

### The [RP2350](https://www.raspberrypi.com/products/rp2350/) is supported by ConfigurableFirmata v3.4 or later. 2025-04-16+ (Pico2 + Pico2W)
| Property  | Value (Logical pin numbers) |
| ----------- | ------------------------------------ |
| Number of Pins  | 48+1* |
| Number of analog inputs | 8+1* |
| *Built in [temperature sensor](https://github.com/earlephilhower/arduino-pico/discussions/2897#discussioncomment-12816194) | A8 |
| Flash Memory | 4Mb? |
| RAM | 520kb |
| PWM capable pins | 24 |
| Built-in LED | Yes, pin 25 |


You can always download the latest version of `Boards.h` file in the `src/utility` folder and overwrite at you PC before compiling.
On Windows, (if you are using Arduino IDE,) it is tipically at: `C:\Users\{username}\Documents\Arduino\libraries\ConfigurableFirmata\src\utility\Boards.h`.
On macOS, this file can be found here by default: `~/Documents/Arduino/libraries/ConfigurableFirmata/src/utility/Boards.h`.

### I2C limitation, if using the old "Arduino Mbed OS RP2040 Boards" library

The Raspberry Pi Pico [datasheet](https://datasheets.raspberrypi.org/pico/Pico-R3-A4-Pinout.pdf) states that the
default GPIOs for I2C-0 are GP 4 (SDA-0) and GP 5 (SCL-0) (physical pins 6 & 7 respectively).
However, v2.3.1 of the Mbed OS RP2040 library defines the default Firmata `Wire` implementation to use
I2C-1 on GP 6 (SDA-1) and GP 7 (SCL-1) (physical pins 9 and 10).

The pins that are used for I2C communication can be changed by editing the `variants/RASPBERRY_PI_PICO/pins_arduino.h` file.
When using v2.3.1 on macOS, this file can found in the `~/Library/Arduino15/packages/arduino/hardware/mbed_rp2040/2.3.1/variants/RASPBERRY_PI_PICO` folder.
Simply update the `PIN_WIRE_SDA` and `PIN_WIRE_SCL` values, for example to use the default I2C-0 pins:

```
#define PIN_WIRE_SDA  (4u)
#define PIN_WIRE_SCL  (5u)
```

Note that while the Pico supports two I2C buses, Firmata currently only supports interfacing with one I2C bus.
Support for two I2C buses can be enabled within the RP2040 toolchain by setting `WIRE_HOWMANY` to `(2)`, and defining `PIN_WIRE_SDA1`, `PIN_WIRE_SCL1`, `I2C_SDA1` and `I2C_SCL1`:

```
#define WIRE_HOWMANY  (2)
#define PIN_WIRE_SDA  (4u)
#define PIN_WIRE_SCL  (5u)
#define PIN_WIRE_SDA1 (6u)
#define PIN_WIRE_SCL1 (7u)
#define I2C_SDA       (digitalPinToPinName(PIN_WIRE_SDA))
#define I2C_SCL       (digitalPinToPinName(PIN_WIRE_SCL))
#define I2C_SDA1      (digitalPinToPinName(PIN_WIRE_SDA1))
#define I2C_SCL1      (digitalPinToPinName(PIN_WIRE_SCL1))
```
