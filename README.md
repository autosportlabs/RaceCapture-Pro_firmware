RaceCapture/Pro Firmware
========================

Firmware for RaceCapture/Pro data acqusition / control / telemetry system

http://www.autosportlabs.net/RaceCapture

See the License.txt file for more information on licensing for this software.

= Flashing =

WARNING: Updating firmware restores the default configuration on RaceCapture/Pro
Be sure to save your configuration using the Race Analyzer software.

================================
Windows
================================

Flash the firmware by double-clicking the flashRCP.bat file in Windows Explorer and follow the instructions.

================================
Linux
================================
While holding down the button on RaceCapture/Pro, plug the unit into USB. You should see 3 green LEDs light up.

Now flash the firmware by executing the flasher application at the command line:

#flasher RaceCapturePro-X.X.X.elf

================================
Flashing process
================================
The flasher application should find the RaceCapture/Pro device and then start flashing new firmware. The red front panel LED on RaceCapture/Pro will flash while firmware is written.

Once the firmware completes flashing, RaceCapture/Pro will reset and run normally with the default configuration.
