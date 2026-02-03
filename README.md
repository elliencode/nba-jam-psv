<h1 align="center">
<img align="center" src="extras/screenshots/banner.png" width="50%"><br>
NBA Jam · PSVita Port
</h1>
<p align="center">
  <a href="#setup-instructions-for-players">How to install</a> •
  <a href="#controls">Controls</a> •
  <a href="#known-issues">Known issues</a> •
  <a href="#credits">Credits</a> •
  <a href="#license">License</a>
</p>

Jam with your favorite stars from all 30 NBA teams in over-the-top, high-flying,
2-on-2 arcade basketball just like you remember it – and like you’ve never seen
before!

This repository contains a loader for **the Android release of NBA Jam v.4.00.80**,
based on the [Android SO Loader by TheFloW][gtasa]. The loader provides
a tailored, minimal Android-like environment to run the official ARMv7
game executable on the PS Vita.

Disclaimer
----------------

**NBA Jam** is copyright &copy; 2011-2020 Electronic Arts Inc.
EA, EA SPORTS and the EA SPORTS logo are trademarks of Electronic Arts Inc.

The NBA and individual NBA member team identifications reproduced on this
product are trademarks and copyrighted designs, and/or other forms of 
intellectual property, that are the exclusive property of NBA Properties, Inc,
and the respective NBA member teams and may not be used, in whole or in part,
without the prior written consent of NBA Properties, Inc. &copy; 2011-2020
NBA Properties, Inc. All rights reserved.

The work presented in this repository is not "official" or produced or
sanctioned by the owner(s) of the aforementioned trademark(s) or any other
registered trademark mentioned in this repository.

This software does not contain the original code, executables, or other
non-redistributable parts of the original game product. The authors of
this work do not promote or condone piracy in any way. To launch and play
the game on their PS Vita devices, users must possess their own legally obtained
copy of the game in the form of .apk and .obb files.

Setup Instructions (For Players)
----------------

To properly install the game, follow these steps precisely:

- Install or update [kubridge][kubridge] and [FdFix][fdfix] by copying
  `kubridge.skprx` and `fd_fix.skprx` to your taiHEN plugins folder
  (usually `ur0:tai`) and adding the following two entries
  to your `config.txt` under `*KERNEL`:

```
  *KERNEL
  ur0:tai/kubridge.skprx
  ur0:tai/fd_fix.skprx
```

```diff
! ⚠️ Don't install `fd_fix.skprx` if you're using the rePatch plugin!
```

- Make sure you have `libshacccg.suprx` in the `ur0:/data/` folder on your
  console. If you don't, use [ShaRKBR33D][shrkbrd] to get it quickly and easily.

- <u>Legally</u> obtain your copy of NBA Jam for Android in the form
  of `.apk` and `.obb` files. This port is tailored for the v.4.00.80 (latest)
  version of the game. Other versions may not work.

    - If you have it installed on your phone, you can
      [get all the required files directly from it][unpack-on-phone]
      or by using any APK extractor that you can find on Google Play.

> ℹ️ Verify that your build is the correct version using **sha1sum** (can also
> be found as an online tool). The sha1sum for `lib/armeabi-v7a/libnbajam.so`
> **must** be `7D230A3A59B0F382DE055B9AF15BC2F3D87E010F`

- Open the `.apk` with any ZIP explorer (like [7-Zip](https://www.7-zip.org/))
  and extract the file `lib/armeabi-v7a/libnbajam.so`
  into `ux0:data/com.eamobile.nbajam_row_wf/` on your Vita. The correct resulting path:
  `ux0:data/com.eamobile.nbajam_row_wf/libnbajam.so`

- Open `main.40080.com.eamobile.nbajam_row_wf.obb` with any ZIP explorer (like [7-Zip](https://www.7-zip.org/))
  and extract all of its context into a separate folder on your computer.

- Install [Total Commander](https://www.ghisler.com/download.htm) and its [PSARC plugin](http://totalcmd.net/plugring/PSARC.html).

- Launch Total Commander and navigate up to the folder you just created.

- Select all the files and directories inside the folder, click on File -> Pack.

- Set `psarc` as Compressor and then click on `Configure` button right below.

- Set `PSARC Version` to `1.3`, `Compression` to `ZLIB` and `Ratio` to `0` and press `OK`

- Press `OK` to launch the compression, it will create a file in `C:\ArchiveName.psarc`. (If you get an error, manually change the location in the command line string `psarc: DESTINATIONFOLDER\ArchiveName.psarc`).

- Rename the newly created psarc archive to `data.psarc` and transfer it to `ux0:data/com.eamobile.nbajam_row_wf/`.

- Install `com.eamobile.nbajam_row_wf.vpk` (from [Releases][latest-release]).

Controls
-----------------

The port features support for physical/gamepad controls. In-game help menu contains
information and tutorials on using them.

Known issues
-----------------

* Loading times when you start a match from the main menu and when you quit from
a match back to the main menu are quite slow (up to 1 minute). Unfortunately,
nothing can be done about that. The game is heavy on disk IO and before switching
to PSARC for data files and some extra optimizations, it took around 40 minutes
to load a single level. How it is right now is the best it can be on this platform.
* Multiplayer doesn't work. I do plan to take another look at it in the future.

Credits
----------------

- [The FloW][flow] for the original .so loader.
- [Rinnegatamante][rinne] for VitaGL.
- [gl33ntwine][gl33ntwine] for SoLoBoP and teaching me everything I know about this.

License
----------------

This software may be modified and distributed under the terms of
the MIT license. See the [LICENSE](LICENSE) file for details.

[cross]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/cross.svg "Cross"
[circl]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/circle.svg "Circle"
[squar]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/square.svg "Square"
[trian]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/triangle.svg "Triangle"
[joysl]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/joystick-left.svg "Left Joystick"
[joysr]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/joystick-right.svg "Left Joystick"
[dpadh]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/dpad-left-right.svg "D-Pad Left/Right"
[dpadv]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/dpad-top-down.svg "D-Pad Up/Down"
[dpadu]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/dpad-up.svg "D-Pad Up"
[dpadd]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/dpad-down.svg "D-Pad Down"
[dpadl]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/dpad-left.svg "D-Pad Left"
[dpadr]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/dpad-right.svg "D-Pad Right"
[selec]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/dpad-select.svg "Select"
[start]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/dpad-start.svg "Start"
[trigl]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/trigger-left.svg "Left Trigger"
[trigr]: https://raw.githubusercontent.com/v-atamanenko/mc3-vita/master/extras/icons/trigger-right.svg "Right Trigger"

[gtasa]: https://github.com/TheOfficialFloW/gtasa_vita
[kubridge]: https://github.com/bythos14/kubridge/releases/
[fdfix]: https://github.com/TheOfficialFloW/FdFix/releases/
[unpack-on-phone]: https://stackoverflow.com/questions/11012976/how-do-i-get-the-apk-of-an-installed-app-without-root-access
[shrkbrd]: https://github.com/Rinnegatamante/ShaRKBR33D/releases/latest
[latest-release]: https://github.com/elliencode/actionsquad-psv/releases/latest

[flow]: https://github.com/TheOfficialFloW/
[rinne]: https://github.com/Rinnegatamante/
[gl33ntwine]: https://github.com/v-atamanenko/
