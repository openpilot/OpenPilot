#!/usr/bin/env python

#  Generate a version blob for
# the OpenPilot firmware.
#
# By E. Lafargue (c) 2011 E. Lafargue & the OpenPilot team
#  Licence: GPLv3
#
###
# Usage:
#   versionblob.py --append --firmware=<Firmware file name> --boardid=<Board code>
#
#    append: if present, then append blob to firmware file directly, otherwise create "blob.bin"
#    firmware: the filename of the firmware binary
#    boardid: as a string, the board code, for example "0401" for CC board version 1.
#           should match the codes in firmware description files.
# 
# We have 100 bytes for the whole description.
#
# Only the first 40 are visible on the FirmwareIAP uavobject, the remaining
# 60 are ok to use for packaging and will be saved in the flash
#
# Structure is:
#  4 bytes: header: "OpFw"
#  4 bytes: GIT commit tag (short version of SHA1)
#  4 bytes: Unix timestamp of compile time
#  2 bytes: target platform. Should follow same rule as BOARD_TYPE and BOARD_REVISION in board define files.
#  26 bytes: commit tag if it is there, otherwise branch name. Zero-padded
#   ---- 40 bytes limit ---
#  20 bytes: SHA1 sum of the firmware.
#  40 bytes: free for now.

import binascii
import os
from time import time
import argparse

# Do the argument parsing:
parser = argparse.ArgumentParser(description='Generate firmware desciption blob')
parser.add_argument('--append', action='store_true')
parser.add_argument('--firmware', help='name of firmware binary file' , required=True)
parser.add_argument('--boardid', help='ID of board model, for example 0401 for CopterControl', required=True)
args = parser.parse_args()
print args

if args.append == True:
	print 'Appending description blob directly to ' + args.firmware
	filename = args.firmware
	file = open(filename,"ab")
else:
	filename = 'blob.bin'
	file = open(filename,"wb")


# Write the magic value:
file.write("OpFw")
# Get the short commit tag of the current git repository.
# Strip it to 8 characters for a 4 byte (int32) value.
# We have no full guarantee of unicity, but it is good enough
# with the rest of the information in the structure.
hs= os.popen('git rev-parse --short=8 HEAD').read().strip()
print "Version: " + hs
hb=binascii.a2b_hex(hs)
file.write(hb)
# Then the Unix time into a 32 bit integer:
print "Date: "  + hex(int(time())).lstrip('0x')
hb = binascii.a2b_hex(hex(int(time())).lstrip('0x'))
file.write(hb)

# Then write board type and board revision
hb = binascii.a2b_hex(args.boardid)
file.write(hb)

# Last: a user-friendly description if it exists in GIT, otherwise
# just "unreleased"
hs = os.popen('git describe --exact-match').read()
if len(hs) == 0 :
	print "Unreleased: get branch name instead"
	hs = os.popen('git branch --contains HEAD').read()

file.write(hs[0:26])
file.write("\0"*(26-len(hs)))

## Now we are at the 40 bytes mark.

## Add the 20 byte SHA1 hash of the firmware:
import hashlib
sha1 = hashlib.sha1()
with open('build/coptercontrol/CopterControl.bin','rb') as f: 
    for chunk in iter(lambda: f.read(8192), ''): 
         sha1.update(chunk)
file.write(sha1.digest())

# Pad will null bytes:
file.write('\0'*40)


file.close()

