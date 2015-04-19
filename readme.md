#ConfigurableFirmata

Firmata is a protocol for communicating with microcontrollers from software on a host computer. The [protocol](https://github.com/firmata/protocol) can be implemented in firmware on any microcontroller architecture as well as software on any host computer software package. The arduino repository described here is a Firmata library for Arduino and Arduino-compatible devices. If you would like to contribute to Firmata, please see the [Contributing](#contributing) section below.


##Usage

ConfigurableFirmata is a version of Firmata that breaks features such as Digital Input, Digital Output, Analog Input, Analog Output, I2C, etc into [individual classes](https://github.com/firmata/ConfigurableFirmata/tree/master/src/utility) making it easier to mix and match standard features with custom features. You configure your sketch by adding or removing various include statements. See the [ConfigurableFirmata.ino](https://github.com/firmata/ConfigurableFirmata/blob/master/examples/ConfigurableFirmata/ConfigurableFirmata.ino) file for an example that includes all available features. You are also able to choose between a Serial connection or an Ethernet connection type.


##Firmata Client Libraries compatible with ConfigurableFirmata
Only a few Firmata client libraries currently support ConfigurableFirmata:

* javascript
  * [https://github.com/jgautier/firmata]
  * [http://breakoutjs.com]
  * [https://github.com/rwldrn/johnny-five]
* perl
  * [https://github.com/ntruchsess/perl-firmata]


##Installation

To install, simply clone ConfigurableFirmata into your `/Arduino/libraries/` directory (where you install other 3rd party libraries).


<a name="contributing" />
##Contributing

If you discover a bug or would like to propose a new feature, please open a new [issue](https://github.com/firmata/ConfigurableFirmata/issues?sort=created&state=open).

To contribute, fork this repository and create a new topic branch for the bug, feature or other existing issue you are addressing. Submit the pull request against the *master* branch.

You must thoroughly test your contributed code. In your pull request, describe tests performed to ensure that no existing code is broken and that any changes maintain backwards compatibility with the existing api. Test on multiple Arduino board variants if possible. We hope to enable some form of automated (or at least semi-automated) testing in the future, but for now any tests will need to be executed manually by the contributor and reviewers.

Use [Artistic Style](http://astyle.sourceforge.net/) (astyle) to format your code. Set the following rules for the astyle formatter:

```
style = ""
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
