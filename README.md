# SPCPlay.exe (2.21+) meets MPRIS

This program first initializes itself as a client discoverable through MPRIS, then launches [spcplay.exe](https://github.com/dgrfactory/spcplay) through [Wine](https://www.winehq.org/) with the given spc file. Why? Since SPCPlay isn't natively recognized as a media player (mainly because it is a *windows executable*), it cannot be controlled by **media keys** on a keyboard without such a bridge to MPRIS. \
<br>
<br>

## Dependencies: sdbus-c++<span style="color:grey"> gcc/g++ make cmake wine spcplay</span>

Included here is the [mpris_server.hpp](https://github.com/chrg127/mpris-server) header-only C++ library, which itself has only one
dependency: [sdbus-c++](https://github.com/Kistler-Group/sdbus-cpp). Building sdbus-c++ requires cmake.

**TL;DR** \
Clone the sdbus-c++ repo and execute:
```
$  mkdir build && cd build
$  cmake .. -DCMAKE_BUILD_TYPE=Release
$  sudo cmake --build . --target install
```

And **that's it.** \
This program also uses a (tiny) homemade header-only c library to read the Super NES's music files metadata.

**Addendum** \
Spcplay must be version 2.21 or higher, such as [this release](https://github.com/dgrfactory/spcplay/releases/tag/2.21.0.8765b). \
<br>

## Setup

Simply clone this repo and run ```make``` in the root folder.
The executable will be in the newly created "build" folder. \
<br>
<br>
***(Optional)* Giving the program a desktop entry and icon:**
 - Fill the missing fields from spcplayer.desktop
 - Move spcplayer.desktop to ~/.local/share/applications/
 - Move spcplayer.png to </your/path/to/an/icon.png>
<!-- TODO: slim (banner) screenshot of the app menu, to see the icon in an example (rofi) -->

<!-- ***(Optional)* Adding a custom SPC file extension to the mimeapps list:** -->
<!-- TODO:  SPC files are, by default, recognized as "application/x-pkcs7-certificates", but we can also give them a more appropriate name, i.e. "audio/spc-dump" -->
<!-- TODO: slim (banner) screenshot of the file's "file -b --mime-type" output -->
<br>

## Usage

```
$  spcplay-mpris <path/to/spcplay.exe> <path/to/file.spc>
```