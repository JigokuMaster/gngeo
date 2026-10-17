NeoGeo emulator for Symbian S60v3 and higher (devices with physical keyboard)

this port is based on [GnGeo 0.8](https://github.com/linuxlinks/gngeo). 

# Installation

see the [releases](https://github.com/JigokuMaster/gngeo/releases) page

* gngeo.sisx: a little slower, it's using the portable CPU cores (gen68k, mamez80)

* gngeo-armv6.sisx: a little faster, it was built using GCCE 4.6.3

* gngeo-cyclone.sisx: it's using the ARM assembly CPU core, more faster, but it cannot load states that have been saved with the previous versions of gngeo.


* you may need to install PIPS 1.7

* it's highly recommended to download games as .gno ROM, this format loads faster and use less RAM compared to zipped ROMs you can download most of the games from [here](https://github.com/steward-fu/website/releases/tag/neogeo)

* put your games in the drive where gngeo was installed, if installed in C then put them in C:\\gngeo\\roms

#  Notes

- gngeo can not play all games (metalslug4, metalslug4 plus, metalslug3, metalslug5) are loadable but not playable.

- if some game is runnig slow enabling frameskip options should help.

- big games need enough free RAM 30MB or more, in this case make sure to download the game as .gno ROM! and keep the sound disabled.

# Controls 

those are the default keymappings, but anyway you should configure to your preferred keys (see  Option > Controls)

Neogeo       |    SYMBIAN
__________________________

Start              :    KEY 5 or ENTER 

Insert Coin        :    KEY 1


A                  :    KEY 2


B                  :    KEY 8


C                  :    KEY 4


D                  :    KEY 6


Joystick           :    ARROW KEYS

use */# Keys to control audio volume. 

use the GREEN Key to take screenshot, it will be saved in gngeo\screenshots folder.

# Building

gngeo was built on Linux (using the gnupoc-package and GCCE 3.4.3) but can be built on windows too using standard S60v3 SDK.

first clone/download [SDL1.2.14](https://github.com/JigokuMaster/symbian-sdl-libs) then build the ESDL.lib

```bash

cd SDL1.2.14_ag/Symbian/s60v3

bldmake bldfiles gcce urel

abld build -v gcce urel
```

clone/download gngeo repo

```bash

cd gngeo

make prebuild

make build

make mksis

```

