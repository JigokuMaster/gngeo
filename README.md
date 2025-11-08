# gngeo

NeoGeo emulator for Symbian S60

based on the original [GnGeo 0.8 ](https://github.com/linuxlinks/gngeo). built with the default SDL launcher. but it still incomplete, for now it can work only on Symbian 9 devices with physical keyboard. 
preferably with 320x240p or  bigger screen. 

first download [gngeo.sisx](https://github.com/JigokuMaster/gngeo/blob/v1/sis/gngeo.sisx) 

put your  roms in the c:\\gngeo\\roms
and if you have an SD card you should put them in e:\\gngeo\\roms 

you also need neogeo bios files. they should be inside neogeo.zip you need to put it in  the roms folder. 

# Controls : 

Neogeo             | SYMBIAN
__________________________

Start            :  KEY 5 or ENTER 

Insert Coin :  KEY 1


A                  :  KEY 2


B                  :  KEY 8


C                  :  KEY 4


D                  :  KEY 6


Joystick     :  ARROW KEYS

use * or # KEY to control audio volume. 

# Notes :
- the sound is disabled by default. you can enable it from samplerate menu. but  the games will run very slow. 

- you may need more RAM to play some big games , GnGeo will exit when there is no enough memory. dumpgfx might be a solution ... 

- GnGeo won't recognize some games,  for example if you have metalslugx.zip , rename it to mslugx.zip. see [romrc](https://github.com/JigokuMaster/gngeo/blob/main/gngeo-0.8/romrc) for the proper name set.


