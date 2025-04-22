# SPCPlay.exe (2.21+) meets MPRIS

This program first initializes itself as a client discoverable through MPRIS, then launches [spcplay.exe](https://github.com/dgrfactory/spcplay) through [Wine](https://www.winehq.org/) with the given spc file. Why? Since SPCPlay isn't natively recognized as a media player (mainly because it is a *windows executable*), it cannot be controlled by **media keys** on a keyboard without such a bridge to MPRIS. \
<br>

## Dependencies: sdbus-c++ (gcc/g++ make cmake wine spcplay)

Included here is the [mpris_server.hpp](https://github.com/chrg127/mpris-server) header-only C++ library, which itself has only one
dependency: [sdbus-c++](https://github.com/Kistler-Group/sdbus-cpp). Building sdbus-c++ requires cmake.

**TL;DR** Copy-paste this in a terminal
```
git clone https://github.com/Kistler-Group/sdbus-cpp.git && \
cd sdbus-cpp && mkdir build && cd build                  && \
cmake .. -DCMAKE_BUILD_TYPE=Release                      && \
sudo cmake --build . --target install                    && \
cd ../.. && rm -rf sdbus-cpp/
```

And **that's it.** \
This program also uses a (tiny) homemade header-only c library to read the Super NES's music files metadata.

**Addendum** \
Spcplay must be version 2.21 or higher, such as [this release](https://github.com/dgrfactory/spcplay/releases/tag/2.21.0.8765b). \
<br>

## Setup

 - Clone this repo
 - In **spcplay-mpris.cpp**, replace the **SPCPLAY_EXE** macro with your path to spcplay.exe (then save)
 - Run the ```make``` command in the root folder!

The executable will be in the newly created "build" folder. \
<br>

***(Optional)* Giving the program a desktop entry and icon:**
 - Fill the missing fields from spcplayer.desktop (then save)
 - Move spcplayer.desktop to ~/.local/share/applications/
 - Move spcplayer.png to </your/path/to/spcplayer.png>
<br>

## Usage

```
spcplay-mpris <path/to/file.spc>
```