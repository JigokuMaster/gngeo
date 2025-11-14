NeoGeo emulator for Symbian S60v3 and higher ( devices with physical keyboard) based on [GnGeo 0.8 ](https://github.com/linuxlinks/gngeo). 

# Installation

* download [gngeo_gcce.sisx](https://github.com/JigokuMaster/gngeo/releases) 

* you may need to install PIPS 1.7

* put your games in the drive where gngeo was installed e.g if installed in C then put them in C:\\gngeo\\roms

#  Notes:

- if games play slow on your phone try to enable autoframeskip from option menu

- the sound is disabled by default, you can enable it from samplerate option but games may run slow. 

- big games needs more RAM unfortunately still no solution for devices with low memory.

- GnGeo won't recognize some games,for example if you have metalslugx.zip , rename it to mslugx.zip. see [romrc](https://github.com/JigokuMaster/gngeo/blob/main/src/gngeo-0.8/romrc) for proper names.


# Controls 

Neogeo       |    SYMBIAN
__________________________

Start            :     KEY 5 or ENTER 

Insert Coin :     KEY 1


A                  :     KEY 2


B                  :     KEY 8


C                  :     KEY 4


D                  :    KEY 6


Joystick     :    ARROW KEYS

use */# Keys to control audio volume. 

use the GREEN Key to take screenshot, it will be saved in gngeo\screenshots folder.

![Main](https://github.com/JigokuMaster/gngeo/raw/main/screenshots/E5_main.jpg)

![PuzzleBobble](https://github.com/JigokuMaster/gngeo/raw/main/screenshots/E5_bp.jpg)


# Building
gngeo was built on linux (using gnupoc package) for ARM (GCCE) WINSCW target wasn't tested.

clone/download [SDL1.2.13](https://github.com/JigokuMaster/symbian-sdl-libs) and build SDL.lib

cd SDL1.2.1/symbian

bldmake bldfiles

abld build -v gcce urel

clone/download gngeo repo 


cd gngeo\group

bldmake bldfiles

abld build -v gcce urel

Linux

```bash

cd gngeo

make prebuild

make build

make mksis

```

# TO-DO:
- Add OpenGLES support.
- Big ROM support for low memory phones
-  Compile without PIPS , maybe a UIQ3.1 port  ...
- Add ROMs downloader