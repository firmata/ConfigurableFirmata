# ConfigurableFirmata

[![Join the chat at https://gitter.im/firmata/ConfigurableFirmata](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/firmata/ConfigurableFirmata?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Firmata is a protocol for communicating with microcontrollers from software on a host computer. The [protocol](https://github.com/firmata/protocol) can be implemented in firmware on any microcontroller architecture as well as software on any host computer software package. The arduino repository described here is a Firmata library for Arduino and Arduino-compatible devices. If you would like to contribute to Firmata, please see the [Contributing](#contributing) section below.

## Installation

- **If you are using Arduino IDE version 1.6.4 or higher** go to `Sketch > Include Library > Manage Libraries` and then search for "ConfigurableFirmata" and click on `Install` after tapping on the ConfigurableFirmata item in the filtered results. You can also use this same method to update ConfigurableFirmata in the future.
- **If you are using an older version of the Arduino IDE**, download or clone ConfigurableFirmata to your Arduino sketchbook library folder. This is typically `/Documents/Arduino/libraries/` on Mac or Linux or `\My Documents\Arduino\libraries\` on Windows.
- **If you want to edit things yourself or help in development**, clone this repo to `\My Documents\Arduino\libraries\ConfigurableFirmata` and start hacking. Just delete the folder if it exists already.

## Release 3.0

ConfigurableFirmata 3.0 contains some internal breaking changes for external modules. If you need to use a particular module (such as FirmataEncoder), either use a 2.xx version or ask the maintainer to update the module to work with V3.0. 

If you have a sketch prepared for an older version of ConfigurableFirmata (< v3.0), you should either restart with the basic example (from the examples directory) or perform the following steps to adjust your sketch to the new library version:

- Remove the line `#include <AnalogWrite.h>`
- Replace the `loop()` function with this snippet:

```cpp
void loop()
{
  while(Firmata.available()) {
    Firmata.processInput();
    if (!Firmata.isParsingMessage()) {
      break;
    }
  }

  firmataExt.report(reporting.elapsed());
}
```

It's no longer necessary to call `update()` for every module that requires polling. That is now handled by the above call to `report`. 

## Usage

ConfigurableFirmata is a version of Firmata that breaks features such as Digital Input, Digital Output, Analog Input, Analog Output, I2C, etc into [individual classes](https://github.com/firmata/ConfigurableFirmata/tree/master/src) making it easier to mix and match standard features with custom features. To start, open the "ConfigurableFirmata.ino" file found in the examples subfolder (or the examples menu of the Arduino IDE). A bunch of `#define` statements at the beginning of the file let you select the modules to include. 

An alternative way to generate the list of included features with ConfigurableFirmata is with [firmatabuilder](http://firmatabuilder.com) which is a simple web application that generates an Arduino sketch based on a selection of Firmata features. Download the generated sketch, compile and upload it to your board.

Another way to use ConfigurableFirmata is by adding or removing various include statements in the [ConfigurableFirmata.ino](https://github.com/firmata/ConfigurableFirmata/blob/master/examples/ConfigurableFirmata/ConfigurableFirmata.ino) example file.

## Supported boards

ConfigurableFirmata is supported on a large variety of boards, both on 8-Bit microcontrollers such as the AVR-Based Arduinos as well as on 32-Bit microcontrollers such as the ESP32, Arduino Due or Raspberry Pi Pico.

For details on particular boards see [this page](BoardSupport.md).

## Firmata Wrapper Libraries

You can use the ConfigurableFirmata architecture to wrap 3rd party libraries to include
functionality not included in the base ConfigurableFirmata.ino example. See [FirmataEncoder](https://github.com/firmata/FirmataEncoder) for an example of a Firmata wrapper. To include a Firmata wrapper in your sketch, you must install both ConfigurableFirmata and the 3rd party library into your `/Arduino/libraries/` directory (where all 3rd party libraries are installed).

When creating a new Firmata wrapper library, you generally should not include the 3rd party
library it wraps. For example, the Encoder library that FirmataEncoder wraps is not included with
the FirmataEncoder library.

If you create a wrapper library, prepend the name with 'Firmata'. Hence 'FirmataEncoder' in the
referenced example. This will keep the wrapper libraries together in the user's Arduino libraries
directory.

## Firmata Client Libraries
Not all client libraries officially support ConfigurableFirmata, but the protocol implementation is the same as for StandardFirmata, so most libraries should work. These have been tested:

* javascript
  * [https://github.com/jgautier/firmata]
  * [https://github.com/rwldrn/johnny-five]
  * [http://breakoutjs.com]
* perl
  * [https://github.com/ntruchsess/perl-firmata]

* java
  * [https://www.diozero.com/]

* C# / .NET
  * [https://github.com/dotnet/iot] - recommended

*Additional Firmata client libraries may work as well. If you're a client library developer and have verified that you library works with ConfigurableFirmata, please [open an issue](https://github.com/firmata/ConfigurableFirmata/issues) with a request to add the link.*

## Contributing

If you discover a bug or would like to propose a new feature, please open a new [issue](https://github.com/firmata/ConfigurableFirmata/issues?sort=created&state=open).

To contribute, fork this repository and create a new topic branch for the bug, feature or other existing issue you are addressing. Submit the pull request against the *master* branch.

You must thoroughly test your contributed code. In your pull request, describe tests performed to ensure that no existing code is broken and that any changes maintain backwards compatibility with the existing api. Test on multiple Arduino board variants if possible. We hope to enable some form of automated (or at least semi-automated) testing in the future, but for now any tests will need to be executed manually by the contributor and reviewers.

Use [Artistic Style](http://astyle.sourceforge.net/) (astyle) to format your code. Set the following rules for the astyle formatter:

```
style = ""
indent = "spaces"
indent-spaces = 2
indent-classes = true
indent-switches = true
indent-cases = true
indent-col1-comments = true
pad-oper = true
pad-header = true
keep-one-line-statements = true
```

If you happen to use Sublime Text, [this astyle plugin](https://github.com/timonwong/SublimeAStyleFormatter) is helpful. Set the above rules in the user settings file.
