# Simple PS3 DDR Dancemat Driver for Linux

This is a very simple Linux Kernel module for the PS3 DDR Dancemat.

This driver will properly map the Dancemats arrows to button hits instead of HAT movements.
This allows for LEFT+RIGHT and UP+DOWN hits.
All 11 buttons are mapped.

Installation instructions for Fedora, Ubuntu, and Arch will be provided.

## WARNING!
Although this driver works 100% for it's intended purpose (playing Project Outfox!) the driver
is not feature complete!

Button mappings will change in future versions without warning. I will slap together a proper release (with .deb and .rpm files)
once I'm happy. (I'm not a happy person. So give me a year or so.)

## Installation
(Instuctions are not complete)
### Fedora (34)
Install the build tools if you don't already have them.

`dnf install @development-tools`

Clone this repo and enter it

`git clone https://github.com/UntrustedRoot/Simple-PS3-DDR-Dancemat-Driver-for-Linux.git & cd Simple-PS3-DDR-Dancemat-Driver-for-Linux`

Compile

`make`

Load the module

`sudo insmod usb_ps3_dancemat_driver.ko`

Install the module for future reboots

`TODO`
