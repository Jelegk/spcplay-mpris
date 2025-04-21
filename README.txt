An mpris listener to control a running spcplay.exe (2.21+) (through Wine)




Dependencies: gcc/g++, make, cmake, sdbus-c++ (spcplay, wine)
------------
Included here is the mpris_server.hpp header-only C++ library from
https://github.com/chrg127/mpris-server, which itself has only one
dependency: sdbus-c++ (https://github.com/Kistler-Group/sdbus-cpp).

sdbus-c++'s building requires cmake

TL;DR:
Clone the sdbus-c++ repo and execute 
    $ mkdir build && cd build
    $ cmake .. -DCMAKE_BUILD_TYPE=Release
    $ sudo cmake --build . --target install

Other than that, that's it. This program also uses a (tiny) homemade
header-only c library to read the Super NES's music files metadata.


Addendum
--------
Since spcplay.exe is a windows executable, I doubt I need to mention that Wine
is also a dependency. For installation, please refer to their website's wiki:
https://gitlab.winehq.org/wine/wine/-/wikis/Download

Spcplay must be version 2.21 or higher, such as:
https://github.com/dgrfactory/spcplay/releases/tag/2.21.0.8765b




Setup
-----
Simply clone this repo and run "make" in the root folder.
The executable will be in the newly created "build" folder.


(Optional) Giving the program a desktop icon:
TODO:    add .desktop file, show "heres a list of possible locations (and my fav is ___)"
         slim (banner) screenshot of the app menu, to see the icon in an example (rofi)

(Optional) Adding a custom SPC file extension to the mimeapps list:
TODO: spc files are, by default, recognized as ___, but we can also give them a more appropriate name, i.e. ___
      slim (banner) screenshot of the file in pcmanfm status bar and its "file -b --mime-type" output
