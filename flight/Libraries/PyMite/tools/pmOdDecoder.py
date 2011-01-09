#!/usr/bin/env python

# This file is Copyright 2009, 2010 Dean Hall.
#
# This file is part of the Python-on-a-Chip program.
# Python-on-a-Chip is free software: you can redistribute it and/or modify
# it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.
#
# Python-on-a-Chip is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
# is seen in the file COPYING in this directory.

"""
PyMite Object Descriptor Decoder
================================

Decodes an object descriptor value into its bit fields.
"""

## @file
#  @copybrief pmOldDecoder

## @package pmOldDecoder
#  @brief PyMite Object Descriptor Decoder
#
#  Decodes an object descriptor value into its bit fields.


import sys, pprint


__usage__ = """USAGE:
    ./pmOdDecoder.py odvalue
"""


TYPES = (
    'OBJ_TYPE_NON',
    'OBJ_TYPE_INT',
    'OBJ_TYPE_FLT',
    'OBJ_TYPE_STR',
    'OBJ_TYPE_TUP',
    'OBJ_TYPE_COB',
    'OBJ_TYPE_MOD',
    'OBJ_TYPE_CLO',
    'OBJ_TYPE_FXN',
    'OBJ_TYPE_CLI',
    'OBJ_TYPE_CIM',
    'OBJ_TYPE_NIM',
    'OBJ_TYPE_NOB',
    'OBJ_TYPE_THR',
    0x0E,
    'OBJ_TYPE_BOOL',
    'OBJ_TYPE_CIO',
    'OBJ_TYPE_MTH',
    'OBJ_TYPE_LST',
    'OBJ_TYPE_DIC',
    0x14,0x15,0x16,0x17,0x18,
    'OBJ_TYPE_FRM',
    'OBJ_TYPE_BLK',
    'OBJ_TYPE_SEG',
    'OBJ_TYPE_SGL',
    'OBJ_TYPE_SQI',
    'OBJ_TYPE_NFM',
)


def od_decode(odvalue):
    return {
        "val": odvalue,
        "size": (odvalue & 0x001F) * 4,
        "type": TYPES[(odvalue & 0x3E00) >> 9],
        "mark": (odvalue & 0x4000) >> 14,
        "free": (odvalue & 0x8000) >> 15,
    }


def to_int(s):
    if s.startswith("0x"):
        return int(s, 16)
    return int(s)


def main():
    odvalues = sys.argv[1:]
    odvalues = map(to_int, odvalues)
    ods = map(od_decode, odvalues)
    for od in ods:
        print("%d (0x%04x): %s[%d], f=%d, m=%d"
            % (od['val'], od['val'], od['type'], od['size'], od['free'], od['mark']))


if __name__ == "__main__":
    main()
