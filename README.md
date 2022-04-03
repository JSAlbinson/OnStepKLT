OnStep Telescope Controller
===========================

This is a fork of the OnStep system for the Keele University Observatory Thornton 24in Telescope. It is specific to this telescope and is unlikely to be of interest to other telescope nuts except as an interesting oddity. This repository is private for the time being as I and a few colleagues work on it. When it is fit to be seen it will be made public. The telescope has 5 motors: 3 large steppers (2 x Nema 34, plus 1x large Nema 23) and 2 3phase mains induction motors wired as reversible single phase motors. It has a clutch system to switch between slew and track on RA, and a safety brake system to hold the telescope safe in power down mode. The object of this exercise is to insert some code wedges to cater for the slew situations, and leave the rest of the code base alone. The telescope is a German Equatorial Mount (GEM), weighing 2 tons total, with a half ton counterweight and half ton telescope frame at a moment arm of 1 metre. The optics are a 603mm f4.45 Newtonian system feeding (currently) a QSI 583 CCD camera.

There are three additions to be made:

When "slew" or "9" or "max" or anything equivalent is selected on the SHC, or the main control screen on the PC, the slew/track pin is selected HIGH and the appropriate slew motor/direction pin is made HIGH. The clutch select pin splits into a direct connect to the slew clutch, and also a connection through a logic inverter to the track clutch. This means when slew is deselected, track is selected. When system power is on, the track clutch is selected by default. Slew is only selected when needed. Only when the button on the handset or the virtual h/s is released does the slew stop. The inbuilt software limits on telescope position must be honoured.
When a 'goto' of more than 1 degree is requested by whatever means, from the SHC or the main screen, the slew clutch and appropriate slew motor must be selected for an integer number of 10 milliseconds. This is a half cycle of 50Hz mains. We are using zero point crossing mains relays to switch the slew motors to minimise transients. The clutch relays (24V DC) must also be activated for the exact same time.
The safety brake must be energised on power on, but only as the track/slew clutch is also activated. If the soft panic stop is pressed, the safety brake must also be energised off. As this brake is actually a permanent magnet, on power off it clamps the whole telescope in RA, preventing an out-of-balance runaway.
All else is to be left intact as we actually use the functionality.


# Important Note

THERE ARE SEVERAL GITHUB BRANCHES OF ONSTEP:
* The **RELEASE BRANCHES** are well tested and what most should use.  Usually the newest (highest revision) RELEASE is recommended.  No new features are added and only bug fixes where necessary and safe.
* Tne **BETA BRANCH**, if present, is a "snap-shot" of the MASTER where we have reached a point of apparent stability.  This provides access to most new features for adventurous users.
* The **MASTER BRANCH** is the most up to date OnStep version; where new features are added.  It is the least well tested branch and should only be user by experienced users willing to test for and report bugs.

# What is OnStep?
OnStep is a computerized telescope goto controller, based on Teensy or
Arduino control of stepper motors.

It supports Equatorial Mounts (GEM, Fork, etc.) as well as Alt-Az mounts
(including Dobsonians, and the like.)

OnStep was designed, from the beginning, as a more or less general purpose
system and provisions were made in the firmware to allow for use on a variety
of mounts.

# Features
OnStep supports a wide variety of connection options.  Either two or three serial
"command channels" can be utilized. One of the these is normally devoted to a USB
connection and for the other(s) choose from the following:

* Bluetooth
* ESP8266 WiFi
* Arduino M0/Ethernet Shield
* Even another USB port or RS232 serial isn't very difficult to add.

Other software in the OnStep ecosystem include:

* an [ASCOM](http://ascom-standards.org/) driver (with IP and Serial support),
* an Android App useable over WiFi or Bluetooth equipped Phones/Tablets
  (version 2.3.3 or later),
* a "built-in" website (on the Ethernet and/or WiFi device),
* a full planetarium program that controls all features ([Sky Planetarium](http://stellarjourney.com/index.php?r=site/software_sky)).

OnStep is compatible with the LX200 protocol. This means it can be controlled
from other planetarium software, like: Sky Safari, CdC (even without ASCOM),
Stellarium, etc.

There are also [INDI](http://www.indilib.org/about.html) drivers so it can be used from Linux, with CdC or KStars.

# Documentation
Detailed documentation, including the full set of features, detailed designs for
PCBs, instructions on how to build a controller, how to configure the firmware
for your particular mount, can all be found the [OnStep Group Wiki](https://groups.io/g/onstep/wiki/home).

# Change Log
All the changes are tracking in git, and a detailed list can be accessed using the
following git command:
 
git log --date=short --pretty=format:"%h %ad %<(20)%an %<(150,trunc)%s"

# Support
Questions and discussion should be on the mailing list (also accessible via the
web) at the [OnStep Group](https://groups.io/g/onstep/).

# License
OnStep is open source free software, licensed under the GPL.

See [LICENSE.txt](./LICENSE.txt) file.

# Author
[Howard Dutton](http://www.stellarjourney.com)
