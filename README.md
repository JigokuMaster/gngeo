NeoGeo emulator for Symbian S60v3 and higher (devices with physical keyboard)
port based on [GnGeo 0.8](https://github.com/linuxlinks/gngeo). 

# Installation

* Download [gngeo_gcce.sisx](https://github.com/JigokuMaster/gngeo/releases) 

* You may need to install PIPS 1.7

* It's highly recommended to download games as .gno ROM, this format loads faster and use less RAM compared to zipped ROMs you can download most of the games from [here](https://github.com/steward-fu/website/releases/tag/neogeo)

* Put your games in the drive where gngeo was installed, if installed in C then put them in C:\\gngeo\\roms

#  Notes:

- GnGeo can not play all games (metalslug4, metalslug4 plus, metalslug3, metalslug5) are loadable but not playable.

- If some control key is missing from the menu, download the default [gngeorc](https://github.com/JigokuMaster/gngeo/blob/main/src/gngeo-0.8/gngeorc.symbian) rename to gngeorc and copy to gngeo folder in E or C drive.


- If some game is runnig very slow, try to enable autoframeskip from option menu.

- The sound is disabled by default, you can enable it from samplerate option but games may run slow. 

- Big games needs more RAM, remember to download the game as .gno ROM and keep the sound disabled.


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

![PuzzleBobble](https://github.com/JigokuMaster/gngeo/raw/main/screenshots/E5_pb.jpg)


# Building

gngeo was built on Linux (using gnupoc package and GCCE 3.4.3)

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
- Compile without PIPS (UIQ3.1 port).
- Add ROMs downloader.
