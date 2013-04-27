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

from list import append

class UAVObjectMetadata:
	class UpdateMode:
		PERIODIC = 0 
		ONCHANGE = 1  
		MANUAL = 2 
		NEVER = 3 
	
	class Access:
		READWRITE = 0
		READONLY = 1
	
	def __init__(self, objId):
		self.access = UAVObjectMetadata.Access.READWRITE
		self.gcsAccess = UAVObjectMetadata.Access.READWRITE
		self.telemetryAcked = False
		self.telemetryUpdateMode = UAVObjectMetadata.UpdateMode.MANUAL
		self.telemetryUpdatePeriod = 0
		self.gcsTelemetryAcked = False
		self.gcsTelemetryUpdateMode = UAVObjectMetadata.UpdateMode.MANUAL
		self.gcsTelemetryUpdatePeriod = 0
		self.loggingUpdateMode = 0
		self.loggingUpdatePeriod = UAVObjectMetadata.UpdateMode.MANUAL
		self.objId = objId
		self.read()
	
	def read(self):
		pass
	
	def write(self):
		pass

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
		if ftype == UAVObjectField.FType.FLOAT32:
			if numElements == 1:
				self.value = 0.0
			else:
				self.value = [] 
				for n in range(0, numElements):
					append(self.value, 0.0)
		else: 
			if numElements == 1:
				self.value = 0
			else:
				self.value = [] 
				for n in range(0, numElements):
					append(self.value, 0)
		  
class UAVObject:
	def __init__(self, objId):
		self.metadata = UAVObjectMetadata(objId)
		self.objId = objId
		self.instId = 0
		self.fields = []

	def addField(self, field):
		append(self.fields, field)
	'''
	#
	# Support for getName was removed from embedded UAVO database to save RAM + Flash
	#
	def getName(self):
		"""__NATIVE__
		UAVObjHandle objHandle;
		pPmObj_t nameObj;
		pPmObj_t self;
		pPmObj_t attrs;
		pPmObj_t fieldName;
		pPmObj_t field;
		PmReturn_t retval;
		uint32_t objId;
		const char* name;
		uint8_t const *tmpStr;

		// Get dictionary of class attributes                
		self = NATIVE_GET_LOCAL(0);
		attrs = (pPmObj_t)((pPmInstance_t)self)->cli_attrs;
   
		// Get object ID
		tmpStr = (uint8_t const *)"objId";
		retval = string_new(&tmpStr, &fieldName); PM_RETURN_IF_ERROR(retval);    
		retval = dict_getItem(attrs, fieldName, &field); PM_RETURN_IF_ERROR(retval);       
		objId = ((pPmInt_t) field)->val; 
		
		// Get name
		objHandle = UAVObjGetByID(objId);
		name = UAVObjGetName(objHandle);

		// Create return object
		retval = string_new(name, &nameObj); PM_RETURN_IF_ERROR(retval);
		NATIVE_SET_TOS(nameObj);
		return PM_RET_OK;
		"""
		pass
	'''

	def read(self):
		"""__NATIVE__
		uint8_t numBytes;
		UAVObjHandle objHandle;
		uint32_t objId;
		uint16_t instId;
		pPmObj_t self;
		pPmObj_t attrs;
		pPmObj_t field;
		pPmObj_t fields;
		pPmObj_t fieldName;
		pPmObj_t value;
		PmReturn_t retval;
		uint32_t numFields;
		uint32_t fieldIdx;
		uint32_t dataIdx;
		uint32_t valueIdx;
		uint32_t type;  
		uint32_t numElements;   
		uint8_t const *tmpStr;
		int16_t *tmpInt16;
		int32_t *tmpInt32;
		float *tmpFloat;

		// Get dictionary of class attributes                
		self = NATIVE_GET_LOCAL(0);
		attrs = (pPmObj_t)((pPmInstance_t)self)->cli_attrs;

		// Get object ID
		tmpStr = (uint8_t const *)"objId";
		retval = string_new(&tmpStr, &fieldName); PM_RETURN_IF_ERROR(retval);  
		retval = dict_getItem(attrs, fieldName, &field); PM_RETURN_IF_ERROR(retval);       
		objId = ((pPmInt_t) field)->val; 

		// Get the instance ID
		tmpStr = (uint8_t const *)"instId";
		retval = string_new(&tmpStr, &fieldName); PM_RETURN_IF_ERROR(retval);
		retval = dict_getItem(attrs, fieldName, &field); PM_RETURN_IF_ERROR(retval);      
		instId = ((pPmInt_t) field)->val;    
		
		// Get handle and number of bytes in the object
		objHandle = UAVObjGetByID(objId);
		numBytes = UAVObjGetNumBytes(objHandle);
		uint8_t data[numBytes];
		
		// Read object data
		UAVObjGetInstanceData(objHandle, instId, data);
		
		// Get dictionary of fields
		tmpStr = (uint8_t const *)"fields";
		retval = string_new(&tmpStr, &fieldName); PM_RETURN_IF_ERROR(retval);     
		retval = dict_getItem(attrs, fieldName, &fields); PM_RETURN_IF_ERROR(retval);
		numFields = ((pPmList_t) fields)->length;    

		// Process each field
		dataIdx = 0;
		for (fieldIdx = 0; fieldIdx < numFields; ++fieldIdx)
		{		
			// Get field
			retval = list_getItem(fields, fieldIdx, &field); PM_RETURN_IF_ERROR(retval);
			attrs = (pPmObj_t)((pPmInstance_t)field)->cli_attrs;
			// Get type
			tmpStr = (uint8_t const *)"ftype";
			retval = string_new(&tmpStr, &fieldName); PM_RETURN_IF_ERROR(retval); 
			retval = dict_getItem(attrs, fieldName, &field); PM_RETURN_IF_ERROR(retval);
			type = ((pPmInt_t) field)->val;   
			// Get number of elements
			tmpStr = (uint8_t const *)"numElements";
			retval = string_new(&tmpStr, &fieldName); PM_RETURN_IF_ERROR(retval); 
			retval = dict_getItem(attrs, fieldName, &field); PM_RETURN_IF_ERROR(retval);
			numElements = ((pPmInt_t) field)->val;
			// Get value
			tmpStr = (uint8_t const *)"value";
			retval = string_new(&tmpStr, &fieldName); PM_RETURN_IF_ERROR(retval); 
			retval = dict_getItem(attrs, fieldName, &field); PM_RETURN_IF_ERROR(retval); 
			// Set value for each element
			for (valueIdx = 0; valueIdx < numElements; ++valueIdx)
			{		
				// Update value based on type    	
				switch (type)  
				{
					case TYPE_INT8: 
					case TYPE_UINT8:
					case TYPE_ENUM: 
						retval = int_new(data[dataIdx], &value); PM_RETURN_IF_ERROR(retval);                       
						dataIdx = dataIdx + 1;
						break;
					case TYPE_INT16:
					case TYPE_UINT16:
						tmpInt16 = (int16_t*)(&data[dataIdx]);
						retval = int_new(*tmpInt16, &value); PM_RETURN_IF_ERROR(retval);    
						dataIdx = dataIdx + 2;
						break;       
					case TYPE_INT32:
					case TYPE_UINT32:
						tmpInt32 = (int32_t*)(&data[dataIdx]);
						retval = int_new(*tmpInt32, &value); PM_RETURN_IF_ERROR(retval);    
						dataIdx = dataIdx + 4;
						break;  
					case TYPE_FLOAT32:
						tmpFloat = (float*)(&data[dataIdx]);
						retval = float_new(*tmpFloat, &value); PM_RETURN_IF_ERROR(retval);    
						dataIdx = dataIdx + 4;
						break;    
				}
				// Set value 
				if ( OBJ_GET_TYPE(field) == OBJ_TYPE_LST )
				{
					retval = list_setItem(field, valueIdx, value); PM_RETURN_IF_ERROR(retval); 
				}
				else
				{
					tmpStr = (uint8_t const *)"value";
					retval = string_new(&tmpStr, &fieldName); PM_RETURN_IF_ERROR(retval); 
					retval = dict_setItem(attrs, fieldName, value); PM_RETURN_IF_ERROR(retval); 
				}
			}
		}
		
		// Done
		return PM_RET_OK;
		"""
		pass

	def write(self):
		"""__NATIVE__
		uint8_t numBytes;
		UAVObjHandle objHandle;
		uint32_t objId;
		uint16_t instId;
		pPmObj_t self;
		pPmObj_t attrs;
		pPmObj_t field;
		pPmObj_t fields;
		pPmObj_t fieldName;
		pPmObj_t value;
		PmReturn_t retval;
		uint32_t numFields;
		uint32_t fieldIdx;
		uint32_t dataIdx;
		uint32_t valueIdx;
		uint32_t type;  
		uint32_t numElements;  
		uint8_t const *tmpStr;
		int8_t tmpInt8 = 0;
		int16_t tmpInt16;
		int32_t tmpInt32;
		float tmpFloat;		

		// Get dictionary of class attributes                
		self = NATIVE_GET_LOCAL(0);
		attrs = (pPmObj_t)((pPmInstance_t)self)->cli_attrs;

		// Get object ID
		tmpStr = (uint8_t const *)"objId";
		retval = string_new(&tmpStr, &fieldName); PM_RETURN_IF_ERROR(retval);  
		retval = dict_getItem(attrs, fieldName, &field); PM_RETURN_IF_ERROR(retval);       
		objId = ((pPmInt_t) field)->val; 

		// Get the instance ID
		tmpStr = (uint8_t const *)"instId";
		retval = string_new(&tmpStr, &fieldName); PM_RETURN_IF_ERROR(retval);
		retval = dict_getItem(attrs, fieldName, &field); PM_RETURN_IF_ERROR(retval);      
		instId = ((pPmInt_t) field)->val;    
		
		// Get handle and number of bytes in the object
		objHandle = UAVObjGetByID(objId);
		numBytes = UAVObjGetNumBytes(objHandle);
		uint8_t data[numBytes];
			
		// Get dictionary of fields
		tmpStr = (uint8_t const *)"fields";
		retval = string_new(&tmpStr, &fieldName); PM_RETURN_IF_ERROR(retval);     
		retval = dict_getItem(attrs, fieldName, &fields); PM_RETURN_IF_ERROR(retval);
		numFields = ((pPmList_t) fields)->length;    

		// Process each field
		dataIdx = 0;
		for (fieldIdx = 0; fieldIdx < numFields; ++fieldIdx)
		{		
			// Get field
			retval = list_getItem(fields, fieldIdx, &field); PM_RETURN_IF_ERROR(retval);
			attrs = (pPmObj_t)((pPmInstance_t)field)->cli_attrs;
			// Get type
			tmpStr = (uint8_t const *)"ftype";
			retval = string_new(&tmpStr, &fieldName); PM_RETURN_IF_ERROR(retval); 
			retval = dict_getItem(attrs, fieldName, &field); PM_RETURN_IF_ERROR(retval);
			type = ((pPmInt_t) field)->val;   
			// Get number of elements
			tmpStr = (uint8_t const *)"numElements";
			retval = string_new(&tmpStr, &fieldName); PM_RETURN_IF_ERROR(retval); 
			retval = dict_getItem(attrs, fieldName, &field); PM_RETURN_IF_ERROR(retval);
			numElements = ((pPmInt_t) field)->val;
			// Get value
			tmpStr = (uint8_t const *)"value";
			retval = string_new(&tmpStr, &fieldName); PM_RETURN_IF_ERROR(retval); 
			retval = dict_getItem(attrs, fieldName, &field); PM_RETURN_IF_ERROR(retval); 
			// Set value for each element
			for (valueIdx = 0; valueIdx < numElements; ++valueIdx)
			{
				// Get value
				if ( OBJ_GET_TYPE(field) == OBJ_TYPE_LST )
				{
					retval = list_getItem(field, valueIdx, &value); PM_RETURN_IF_ERROR(retval); 
				}
				else
					value = field;
				// Update value based on type    
				switch (type)  
				{
					case TYPE_INT8: 
					case TYPE_UINT8:
					case TYPE_ENUM:       
						if ( OBJ_GET_TYPE(value) == OBJ_TYPE_INT )  
						{
							tmpInt8 = (int8_t)((pPmInt_t)value)->val;
						}
						else if ( OBJ_GET_TYPE(value) == OBJ_TYPE_FLT )  
						{
						    tmpInt8 = (int8_t)((pPmFloat_t)value)->val;  
						} 
						memcpy( &data[dataIdx], &tmpInt8, 1 );
						dataIdx = dataIdx + 1;
						break;
					case TYPE_INT16:
					case TYPE_UINT16:
						if ( OBJ_GET_TYPE(value) == OBJ_TYPE_INT )  
						{
							tmpInt16 = (int16_t)((pPmInt_t)value)->val;
						}
						else if ( OBJ_GET_TYPE(value) == OBJ_TYPE_FLT )  
						{
						    tmpInt16 = (int16_t)((pPmFloat_t)value)->val;  
						} 					
						memcpy( &data[dataIdx], &tmpInt16, 2 );
						dataIdx = dataIdx + 2;
						break;       
					case TYPE_INT32:
					case TYPE_UINT32:
						if ( OBJ_GET_TYPE(value) == OBJ_TYPE_INT )  
						{
							tmpInt32 = (int32_t)((pPmInt_t)value)->val;
						}
						else if ( OBJ_GET_TYPE(value) == OBJ_TYPE_FLT )  
						{
						    tmpInt32 = (int32_t)((pPmFloat_t)value)->val;  
						} 						
						memcpy( &data[dataIdx], &tmpInt32, 4 );
						dataIdx = dataIdx + 4;
						break;  
					case TYPE_FLOAT32:
						if ( OBJ_GET_TYPE(value) == OBJ_TYPE_INT )  
						{
							tmpFloat = (float)((pPmInt_t)value)->val;
						}
						else if ( OBJ_GET_TYPE(value) == OBJ_TYPE_FLT )  
						{
						    tmpFloat = (float)((pPmFloat_t)value)->val;  
						} 						
						memcpy( &data[dataIdx], &tmpFloat, 4 );
						dataIdx = dataIdx + 4;
						break;    
				}
			}
		}
		
		// Write object data
		UAVObjSetInstanceData(objHandle, instId, data);
		
		// Done
		return PM_RET_OK;
		"""
		pass 






