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
Easy.  Its make!  Just run `make` and it will build it all.

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

### Toolchain Setup
Do the following to setup the MK2 toolchain:
* Download [the official MK2
  toolchain](https://launchpad.net/gcc-arm-embedded/4.7/4.7-2013-q3-update/)
* Extract the tarball to a directory of your choice
* Add the 'bin' directory from within the newly extracted directory to
  your system PATH.
* Install pip (the Python package manager)
* Go to the [ASL F4 Loader
  Project](https://github.com/autosportlabs/ASL_F4_bootloader) and
  follow the instructions on the README.md to install the
  asl_f4_loader application.  This package does all the post-processing on
  MK2 firmware and provides a firmware loading utility.

### Compiling MK2 Firmware
From the root of the project, simply run `make mk2`.  For a package run
`make mk2-package`.

When building from the command line, a debug version of the firmware is
built by default. To disable debug and enable standard optimization,
prefix the make command with:

RELEASE_TYPE=RELEASE_TYPE_BETA

## Test
The Test platform is used to validate and stress test our firmware code.
While not a real platform per se, its easier to treat it this way.  The
test platform will work natively with Linux and OSX (>10.10).

### Toolchain Setup
#### Linux
* Install the 64bit GCC packages for your system (need ld)
* Install libstdc++-static and libstdc++-devel for linking in test applications.
* Install the 64bit libcppunit library and its header and devel packages
* Install glibc-devel and glibc-headers
* `make test`
