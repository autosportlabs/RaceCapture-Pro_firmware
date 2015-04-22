RaceCapturePro Firmware
=====

The premier open source telemetry system for your race vehicle.  RaceCapturePro is a
device that gets installed into your race vehicle and beams telemetry about your
vehicle and driving habits in real time.  Combined with
[RaceCapture App](https://github.com/autosportlabs/RaceCapture_App) and
[RaceCapture Live](https://race-capture.com/) a driver and their pit crew can
get the information and extra edge needed to win.

<!--
Inspiration
-----
WE SHOULD DO THIS.
-->

# Overview

As of this writing there are two versions of RaceCapture hardware in the wild.

## MK1

MK1 was the original RaceCapturePro unit.  Based on the AT91SAM7s chipset, it provided
a solid platform for our initial release.  It allowd for up to a 100Hz channel sampling
rate and up to 10Hz GPS.  It also supported CAN & OBD2, various I/O, an SD card, and
a remote GPS mouse for high quality GPS reception.  It came with a separate telemetry
unit that would handle passing the data over the Cellular network.  It however has now
been succedded by the next generation of RaceCapturePro

## MK2

MK2 is the latest RaceCapturePro unit.  Like its successor it has many of the same features
that were originally loved, all of which have been improved upon.  It also has some new features
that were unavialable in the original MK1.  MK2 supports sampling rates of up to 1000 Hz per
channel (a 10x improvement from MK1) and a GPS unit that can sample up to 50Hz (a 5x improvement
from MK1).  It also includes a better Inertial unit, an upgraded processor (STM32 base), more RAM
(for better LUA support) and an integrated cellular device (optinal).  The GPS unit is also integrated
into this unit, allowing for a thinner wire and adjustable antenna (for those needing higher signal
gain in wooded areas).

# Building

The only fully supported building system for RaceCapture Firmware is most any modern Linux based
operating system.  Known systems that work are

* Fedora (x86_64) >= 18
* Ubuntu (x86_64) >= 14.04

There are likely others not listed here, you just need a system that meets the requirements for
our various processor toolchains.

## MK1 Toolchain

** TODO When my Linux Box is present **

## MK2 Toolchain

<!--
===============================
Mk2 Building Requirements
===============================
* Python2.7
* [GCC Arm Embedded 4.7](https://launchpad.net/gcc-arm-embedded)
* [XBVC](https://github.com/Jeff-Ciesielski/XBVC)
* [ihexpy](https://github.com/Jeff-Ciesielski/ihexpy)
* ASL-F4-Loader host utils
    * Clone the
      [ASL_F4_Loader](https://github.com/autosportlabs/ASL_F4_bootloader)
	* Build it using the instructions provided
	* Install the asl_f4_loader_x_x_x.tgz package with pip
-->

** TODO When I don't have friends bekoning me to have a beer **

## Unit Test Toolchain

Unit tests are supported on both modern Linux based systems and on OSX.  The unit tests for
RaceCapturePro are done using clang + libcppunit.  Setting them up is as follows:

### Setup
#### Linux
* Install the 64bit clang packages for your system
* Install the 64bit GCC packages for your system (need ld)
* Install libstdc++-static and libstdc++-devel for linking in test applications.
* Install the 64bit libcppunit library and its header and devel packages
* Install glibc-devel and glibc-headers

#### OSX
* Install the OSX clang compiler.  This is typically done by installing command line tools from XCode.
* Use brew to install libcppunit by executing the following: `brew install cppunit`

### Compiling

Follow these steps to compile the unit tests:

* cd test
* make

### Testing

Simply run `./rcptest` from within the 'test' directory.  That will run the suite of unit test.

# Flashing

Firmware for RaceCapture/Pro data acqusition / control / telemetry system

http://www.autosportlabs.net/RaceCapture

See the LICENSE file for more information on licensing for this software.

WARNING: Updating firmware restores the default configuration on RaceCapture/Pro
Be sure to save your configuration using the Race Analyzer software.

## Windows

Flash the firmware by double-clicking the flashRCP.bat file in Windows Explorer and follow the instructions.

## Linux

While holding down the button on RaceCapture/Pro, plug the unit into USB. You should see 3 green LEDs light up.

Now flash the firmware by executing the flasher application at the command line:

`flasher RaceCapturePro-<VERSION>.elf`

Flashing process
-----
The flasher application should find the RaceCapture/Pro device and then start flashing new firmware. The red front panel LED on RaceCapture/Pro will flash while firmware is written.

Once the firmware completes flashing, RaceCapture/Pro will reset and run normally with the default configuration.


* To flash, enter the stm32_base directory and create a symlink (or copy) the openocd config file matching the adpater you are using. if you have an unsupported adapter, consult the openocd documentation for an appropriate configuration script.

===============================
Steps before submitting a pull request
===============================
* Ensure any 3rd party code you are adding is compatible with our license
* Add appropriate unit tests as needed. Compile and run the tests, ensuring they pass (no regressions)
* run the ./do_style.sh script on the entire source tree to ensure source files are correctly formatted
* submit PR and do a barrel roll!
