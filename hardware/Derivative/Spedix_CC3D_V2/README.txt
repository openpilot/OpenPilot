** Introduction **

These are all the hardware files for the OpenPilot CopterControl 3D, as re-manufactured by Spedix. The files
represent complete schematic designs, and includes all Altium version 9 files. The work is based on the orignal
CC3D design files located at: https://reviews.openpilot.org/browse/OpenPilot/hardware/Production/CopterControl%203D

** License **

These files are licensed under Creative Commons BY-SA 3.0 license. This license also that credit is given, in 
this case the OpenPilot logo should be retained at the same size as on the included files on any board produced 
or hardware derived from this design (which is a derivative of the original work). 

For more details, please see: http://creativecommons.org/licenses/by-sa/3.0/au/deed.en

OpenPilot is a non-commercial project and releasing the original files to share with the wider community, 
please respect this and play fair. If there are any questions or items that need clarifying please contact
one of the OpenPilot forum administrators. 

Spedix has mofified the original design to include new connectors on the board for both Spectrum and S.Bus RX
packages. This board can easily be identified by the added connectors as well as the Gold ENIG Openpilot logo.

This board was orignally shipped alongside the Spedix S250 ARF & BNF kits as sold by BuddyRC. 

http://www.rcgroups.com/forums/showthread.php?t=2341341
http://www.spedix-rc.com/index.php/multirotor/250-series.html
http://www.buddyrc.com/spedix-s250-arf-kit-cc3d-version.html
http://www.buddyrc.com/spedix-s250-bnf-cc3d-version.html

Additionally the board was sold standalone via BuddyRC. Other vendors may be reselling this board, however there
is not a known list of potential resellers at this time. 

http://www.buddyrc.com/cc3d-flight-control-board.html

Please note that the Gyro has been rotated on the board to make access to the USB port easier for end users with 
Mini frames. Pay attention to the arrow when mounting the board. This change eliminates the need for virtual 
rotation of the board within the software. 

Also note that the first batches of this board shipped with an invalid bootloader. The verison number shows up 
as "-128". This bootloader seems to have timing issues which prevent it from being used with newer variants of 
the OpenPilot GCS. You will have to manually update the bootloader per the following instructions:

https://wiki.openpilot.org/display/WIKI/Bootloader+Update

** Caveats ** 

It has been pointed out by Failsafe(Jim) that the board appears to lack ESD protections.

** Credits **

David Ankers: concept, design and board creation. 
James Cotton: Firmware development, 3C algorythm, testing and advice. 
Cathy Moss: Design review, for being generally awesome and having patience for David's questions. 
Kevin Finisterre: Facilitating the exchange of CCBYSA materials from Spedix via BuddyRC, & creating the README.
Dale Deng: Owner of BuddyRC, personally donating for all imported Spedix boards. Helping to obtain Spedix donations.
Spedix: Modification of original design to incorperate S.Bus & Spectrum adaptor as well as rotation of Gyro. 
