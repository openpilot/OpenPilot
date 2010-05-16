__all__ = ("UAVObject")

##
##############################################################################
#
# @file       uavobject.py
# @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
# @brief      Base classes for python UAVObject
#   
# @see        The GNU Public License (GPL) Version 3
#
#############################################################################/
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

import struct
from collections import namedtuple

class UAVMetaData(object):
    def __init__(self):
        self.telemetryAcked = false
        self.telemetryUpdateMode = 0
        self.telemetryUpdatePeriod = 0
        self.gcsTelemetryAcked = false
        self.gcsTelemetryUpdateMode = 0
        self.gcsTelemetryUpdatePeriod = 0
        self.loggingUpdateMode = 0
        self.loggingUpdatePeriod = 0

class UAVObject(object):
    def __init__(self, objid, name, metaname, instanceid, issingle):
        self.objid      = objid
        self.name       = name
        self.metaname   = metaname
        self.instanceid = instanceid
        self.issingle   = issingle

        self.fields     = []

        uavobjects[objid] = self

    def add_field(self, field):
        self.fields.append(field)

    def get_struct(self):
        fmt = "<"
        for f in self.fields:
            fmt += f.get_struct()
        return struct.Struct(fmt)

    def get_tuple(self):
        fieldnames = ' '.join([f.name for f in self.fields])
        return namedtuple (self.name, fieldnames)

    def get_size(self):
        return self.get_struct().size

class UAVObjectField(object):
    def __init__(self, fieldname, type, nelements, elemnames, values):
        self.name      = fieldname
        self.type      = type
        self.nelements = nelements
        self.elemnames = elemnames
        self.values    = values

    def get_struct(self):
        fmt = ""
        if self.nelements > 1:
            fmt += "%u" % self.nelements
        fmt += "%s" % (self.type)
        return fmt

# List of registered uavobject instances
uavobjects = {}

def main():
    pass

if __name__ == "__main__":
    #import pdb ; pdb.run('main()')
    main()
