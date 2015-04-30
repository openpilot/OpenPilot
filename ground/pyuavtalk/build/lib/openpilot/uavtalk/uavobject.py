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

"""__NATIVE__
#include "openpilot.h"

#define TYPE_INT8 0
#define TYPE_INT16 1
#define TYPE_INT32 2
#define TYPE_UINT8 3
#define TYPE_UINT16 4
#define TYPE_UINT32 5
#define TYPE_FLOAT32 6
#define TYPE_ENUM 7

"""

#from list import append

import threading
import struct


class UAVObjectField:
	class FType:
		INT8 = 0
		INT16 = 1
		INT32 = 2
		UINT8 = 3
		UINT16 = 4
		UINT32 = 5
		FLOAT32 = 6
		ENUM = 7
		 
	def __init__(self, ftype, numElements):
		
		self.ftype = ftype
		self.numElements = numElements
		
		if self.ftype == UAVObjectField.FType.INT8:
			vfmt = "b"
			self.rawSizePerElem = 1
		elif self.ftype == UAVObjectField.FType.UINT8 or self.ftype == UAVObjectField.FType.ENUM:
			vfmt = "B"
			self.rawSizePerElem = 1
		elif self.ftype == UAVObjectField.FType.INT16:
			vfmt = "h"
			self.rawSizePerElem = 2
		elif self.ftype == UAVObjectField.FType.UINT16:
			vfmt = "H"
			self.rawSizePerElem = 2
		elif self.ftype == UAVObjectField.FType.INT32:
			vfmt = "i"
			self.rawSizePerElem = 4
		elif self.ftype == UAVObjectField.FType.UINT32:
			vfmt = "I"
			self.rawSizePerElem = 4
		elif self.ftype == UAVObjectField.FType.FLOAT32:
			vfmt = "f"
			self.rawSizePerElem = 4
		else:
			raise ValueError()
		fmt = "<" + vfmt*numElements
		self.struct = struct.Struct(fmt)
		
		self.fmt = fmt
		
		if ftype == UAVObjectField.FType.FLOAT32:
			baseValue = 0.0
		else:
			baseValue = 0
			
		if numElements == 1:
			self.value = baseValue
		else:
			self.value = [baseValue]* numElements
				
		self.rawSize = self.rawSizePerElem * self.numElements
					
	def getRawSize(self):
		return self.rawSize
	
	def serialize(self, ser):
		if self.numElements == 1:
			ser += map(ord, self.struct.pack(self.value))
		else:
			ser += map(ord, apply(self.struct.pack, self.value))
		
	def deserialize(self, data):
		# DOTO: FIXME: This is getting very messy
		values = list(self.struct.unpack("".join(map(chr,data[:self.rawSize]))))
		if self.numElements == 1:
			self.value = values[0]
		else:
			self.value = values
		return self.rawSize
	   
class Observer(object):
	def __init__(self, obj, method):
		self.methodToCall = getattr(obj, method)
		
	def call(self, *args):
		self.methodToCall(args)			
		  
class UAVObject:
	def __init__(self, objId, name=None):
		self.metadata = self   # FIXME
		self.objId = objId
		self.instId = 0
		self.fields = []
		self.observers = []
		self.updateEvent = threading.Condition()
		self.updateCnt = 0
		self.name = name
		self.objMan = None
		
	def updated(self):
		self.objMan.objLocallyUpdated(self)	
	
	
	def waitUpdate(self, timeout=5):
		self.objMan.waitObjUpdate(self, request=False, timeout=timeout)
	
	def getUpdate(self, timeout=.1):
		self.objMan.waitObjUpdate(self, request=True, timeout=timeout)
								
	def addField(self, field):
		self.fields.append(field)
		
	def getSerialisedSize(self):
		size = 0
		for field in self.fields:
			size += field.getRawSize()
		return size
	
	def serialize(self):
		ser = []
		for field in self.fields:
			field.serialize(ser)
		return ser
	
	def deserialize(self, data):
		p = 0
		for field in self.fields:
			p += field.deserialize(data[p:])

	def getName(self):
		pass

	def read(self):
		pass

	def write(self):
		pass 

	def getMetaObjId(self):
		return self.objId + 1 # FIXME, should be in UAVDataObject
	
	def isMetaData(self):
		return False
	
	def __str__(self):
		if self.name != None:
			if self.isMetaData():
				return "%s" % self.name
			else:
				return "%s" % self.name
		else:	
			if self.isMetaData():
				return "UAVMetaObj of %08x" % (self.objId-1)
			else:
				return "UAVObj %08x" % self.objId

#class UAVDataObject(UAVObject):
#	pass # TODO
	
	

class MetaFlightAccessField(UAVObjectField):
	def __init__(self):
		UAVObjectField.__init__(self, UAVObjectField.FType.ENUM, 1)
		
class MetaGCSAccessField(UAVObjectField):
	def __init__(self):
		UAVObjectField.__init__(self, UAVObjectField.FType.ENUM, 1)

class MetaFlightTelemetryAcked(UAVObjectField):
	def __init__(self):
		UAVObjectField.__init__(self, UAVObjectField.FType.ENUM, 1)

class MetaFlightTelemetryUpdateMode(UAVObjectField):
	def __init__(self):
		UAVObjectField.__init__(self, UAVObjectField.FType.ENUM, 1)

class MetaFlightTelemetryUpdatePeriod(UAVObjectField):
	def __init__(self):
		UAVObjectField.__init__(self, UAVObjectField.FType.UINT32, 1)
		
class MetaGCSTelemetryAcked(UAVObjectField):
	def __init__(self):
		UAVObjectField.__init__(self, UAVObjectField.FType.ENUM, 1)

class MetaGCSTelemetryUpdateMode(UAVObjectField):
	def __init__(self):
		UAVObjectField.__init__(self, UAVObjectField.FType.ENUM, 1)

class MetaGCSTelemetryUpdatePeriod(UAVObjectField):
	def __init__(self):
		UAVObjectField.__init__(self, UAVObjectField.FType.UINT32, 1)
		
class MetaLoggingUpdateMode(UAVObjectField):
	def __init__(self):
		UAVObjectField.__init__(self, UAVObjectField.FType.ENUM, 1)

class MetaLoggingUpdatePeriod(UAVObjectField):
	def __init__(self):
		UAVObjectField.__init__(self, UAVObjectField.FType.UINT32, 1)
		

class UAVMetaDataObject(UAVObject):
	class UpdateMode:
		PERIODIC = 0 
		ONCHANGE = 1  
		MANUAL = 2 
		NEVER = 3 
	
	class Access:
		READWRITE = 0
		READONLY = 1
	
	def __init__(self, objId):
		UAVObject.__init__(self, objId)
		
		self.access = MetaFlightAccessField()
		self.addField(self.access)
		self.gcsAccess = MetaGCSAccessField()
		self.addField(self.gcsAccess)
		
		self.telemetryAcked = MetaFlightTelemetryAcked()
		self.addField(self.telemetryAcked)
		self.telemetryUpdateMode = MetaFlightTelemetryUpdateMode()
		self.addField(self.telemetryUpdateMode)
		self.telemetryUpdatePeriod = MetaFlightTelemetryUpdatePeriod()
		self.addField(self.telemetryUpdatePeriod)
	
		self.gcsTelemetryAcked = MetaGCSTelemetryAcked()
		self.addField(self.gcsTelemetryAcked)
		self.gcsTelemetryUpdateMode = MetaGCSTelemetryUpdateMode()
		self.addField(self.gcsTelemetryUpdateMode)
		self.gcsTelemetryUpdatePeriod = MetaGCSTelemetryUpdatePeriod()
		self.addField(self.gcsTelemetryUpdatePeriod)
		
		self.loggingUpdateMode = MetaLoggingUpdateMode()
		self.addField(self.loggingUpdateMode)
		self.loggingUpdatePeriod = MetaLoggingUpdatePeriod()
		self.addField(self.loggingUpdatePeriod)
		
	def isMetaData(self):
		return True
		




