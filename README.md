# Simple PS3 DDR Dancemat Driver for Linux

This is a very simple Linux Kernel module for the PS3 DDR Dancemat.

This driver will properly map the Dancemats arrows to button hits instead of HAT movements.
This allows for LEFT+RIGHT and UP+DOWN hits.
All 11 buttons are mapped.

Installation instructions for Fedora, Ubuntu, and Arch will be provided.

## WARNING!
Although this driver works 100% for it's intended purpose (playing Project Outfox!) the driver
is not feature complete!

## Installation
### DKMS (Recommended)
Use DKMS to have the drivers rebuilt on kernel upgrades, and as an easier way to install.

`chmod +x dkms-install.sh && sudo ./dkms-install.sh`
