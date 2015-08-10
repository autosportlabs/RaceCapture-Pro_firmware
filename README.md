RaceCapture/Pro Firmware
=====
The premier open source telemetry system for your race vehicle.
RaceCapture/Pro is a hardware device that is installed in your vehicle
that beams driving and vehicle telemetry to the cloud in real
time. Combined with [RaceCapture
App](https://github.com/autosportlabs/RaceCapture_App) and
[RaceCapture Live](https://race-capture.com/), teams and drivers can
use the platform to learn how to drive faster an drive better, giving
them the edge to win.


# Contributing
Please checkout the [contributing document](/CONTRIBUTING.md) and
follow all rules in there prior to submitting pull requests.


# Development
The only fully supported building system for RaceCapture Firmware is
most any modern Linux based operating system.

## Working Development Platforms
The following are platforms that are known to work:
* Fedora (x86_64) >= 18
* Centos >= 7
* Ubuntu (x86_64) >= 14.04
* Debian >= Jessie

## Vagrant Development Image
We will also support development on non-linux machines through the use
of a Vagrant image.  This will allow any developer with a computer
capable of running virtualization to compile firmware.

## Flashing Custom Firmware
Updating firmware to unreleased versions may restore the default
configuration on RaceCapture/Pro Be sure to save your configuration
using the Race Analyzer software.  We will do what we can to help
but do understand that this is unreleased code, and sometimes it has
bugs.

# Platforms
The project is designed with multiple platforms in mind.  Most
platforms are hardware based but the testing platform is a pseudo
platform that we use for mock operations to test our firmware.  Info
on each platform and how to use/compile for it can be found below.

## Simple Build All Tool
To make things super simple there is a script called do_release.sh
that will build all platforms and test.  To invoke it simply run
`./do_release.sh` from the root directory of the project.

## MK1
MK1 is the original RaceCapture/Pro unit.  Based on the AT91SAM7s
chipset, it provided a solid platform for our initial release.  It
allows for up to a 100Hz channel sampling rate and up to 10Hz GPS
sampling.  It also supports CAN & OBD-II via the extrernal CANx
module, various I/O, an SD card, and a remote GPS mouse for high
quality GPS reception.  It optionally comes with a separate telemetry
unit that can handle passing the data over the cellular network.  It,
has now been succedded by the next generation of RaceCapture/Pro, MK2.

### Toolchain Setup
Do the following to setup the MK1 toolchain
* Download [the official MK1
  toolchain](https://s3-us-west-2.amazonaws.com/asl-firmware/vagrant_setup/gnuarm-4.0.2.tgz)
* Extract the tarball to a directory of your choice
* Add the 'bin' directory from within the 'gnuarm-4.0.2' directory to
  your system PATH.

### Compiling LUA for MK1
Before compiling the firmware you must compile the LUA library for MK1.  Do the following from the root of the project
* `cd lib_lua/src/`
* `make PLAT=stm32 generic`

### Compiling MK1 Firmware
* run `make PLAT=sam7x` from the root directory of the RCP firmware
  project.

### Flashing
The output from the compilation should create a main.elf file.  The
flashing utility can be found in SAM7s_base/installer.

#### Flashing Mode
Before you can flash the firmware you _MUST_ put the MK1 device into
flashing mode.  Do the following:

* Ensure your MK1 is powered down.
* Hold down the button on RaceCapture/Pro
* Plug the MK1 unit into USB and release the button. You should see 3 green LEDs light up.

#### Flashing in Linux
Perform the following steps from the root level of the project:
* Put the MK1 into programming mode by holding down the button while
  applying power.
* `sudo SAM7s_base/installer/flasher main.elf`


## MK2
MK2 is the second generation RaceCapture/Pro unit.  Like its successor
it has many of the same features that were originally loved, all of
which have been improved upon.  It also has some new features that
were unavailable in the original MK1.  MK2 supports sampling rates of
up to 1 Kilo Hertz per channel (a 10x improvement from MK1) and a GPS unit
that can sample up to 50Hz (a 5x improvement from MK1).  It also
includes a betteriInertial unit, an upgraded processor (STM32 base),
more RAM (for better LUA support) and an integrated cellular device
(optinal).  The GPS unit is also integrated into this unit, allowing
for a thinner wire and adjustable antenna (for those needing higher
signal gain in wooded areas).

### MK2 Toolchain Setup
Do the following to setup the MK2 toolchain:
* Download [the official MK2
  toolchain](https://s3-us-west-2.amazonaws.com/asl-firmware/vagrant_setup/gcc-arm-none-eabi-4_7-2013q1.tgz)
* Extract the tarball to a directory of your choice
* Add the 'bin' directory from within the newly extracted directory to
  your system PATH.
* Install pip (the Python package manager)
* Run `sudo pip install crcmod https://s3-us-west-2.amazonaws.com/asl-firmware/vagrant_setup/asl_f4_loader-0.0.6.tgz`
to install the asl_f4_loader package.  This does post-processing on MK2 firmware and provides a firmware loading
utility.

### Compiling LUA library for MK2
Before compiling the firmware you must compile the LUA library for MK2.  Do the following from the root of the project
* `cd lib_lua/src/`
* `make PLAT=stm32 generic`

### Compiling MK2 Firmware
From the root of the project do the following:
* `cd stm32_base`
* `make PLAT=stm32`


## Test
The Test platform is used to validate and stress test our firmware code.
While not a real platform per se, its easier to treat it this way.  The
test platform will work natively with Linux and OSX (>10.10).

### Toolchain Setup of Test
#### Linux
* Install the 64bit clang packages for your system
* Install the 64bit GCC packages for your system (need ld)
* Install libstdc++-static and libstdc++-devel for linking in test applications.
* Install the 64bit libcppunit library and its header and devel packages
* Install glibc-devel and glibc-headers

#### OSX
* Install the OSX clang compiler.  This is typically done by installing command line tools from XCode.
* Use brew to install libcppunit by executing the following: `brew install cppunit`

### Compiling Test Applications
From the root of the project do the following:
* `cd test`
* `make`

### Running the Unit Tests
From the root of the project do the following:
* `test/rcptest`
