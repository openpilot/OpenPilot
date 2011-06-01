##
##############################################################################
#
# @file       objectManager.py
# @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
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



import logging
import sys
import os
import inspect

from openpilot.uavtalk.uavobject import *



class TimeoutException(Exception): 
    pass 

class ObjManager(object):
    
    def __init__(self, uavTalk):
        self.objs = {}
        self.uavTalk = uavTalk
        uavTalk.setObjMan(self)
        
        
    def addObj(self, obj):
        obj.objMan = self
        self.objs[obj.objId] = obj
        
    def getObj(self, objId):
        try:
            return self.objs[objId]
        except KeyError:
            return None
        
    def getObjByName(self, name):
        for objId, obj in self.objs.items():
            if obj.name == name:
                return obj
        return None
        
    def importDefinitions(self, uavObjDefPath=None):
        # when the uavObjDefPath is nor defined, assume it is installed together with this module
        if uavObjDefPath == None:
            currModPath = os.path.dirname(sys.modules[__name__].__file__)
            uavObjDefPath = os.path.join(currModPath, "..", "uavobjects")
        
        logging.info("Importing UAVObject definitions from %s" % uavObjDefPath)
        sys.path.append(uavObjDefPath)
        for fileName in os.listdir(uavObjDefPath):
            if fileName[-3:] == ".py":
                logging.debug("Importing from file %s", fileName)
                module = __import__(fileName.replace(".py",""))
                for name in dir(module):
                    klass = getattr(module, name)
                    obj = getattr(module, name)
                    if inspect.isclass(obj):
                        if name != "UAVObject"  and name != "UAVMetaDataObject"  and name != "UAVDataObject"  and issubclass(klass, UAVObject):
                            logging.debug("Importing class %s", name)
                            obj = klass()
                            obj.name = name
                            setattr(self, name, obj)
                            self.addObj(obj)
                            metaObj = UAVMetaDataObject(obj.getMetaObjId())
                            obj.metadata = metaObj
                            metaObj.name = "Meta[%s]" % name
                            self.addObj(metaObj)
    
    def regObjectObserver(self, obj, observerObj, observerMethod):
        o = Observer(observerObj, observerMethod)
        obj.observers.append(o)
        
    def objUpdate(self, obj, rxData):
        obj.deserialize(rxData)
        obj.updateCnt += 1
        for observer in obj.observers:
            observer.call(obj)
        obj.updateEvent.acquire()
        obj.updateEvent.notifyAll()
        obj.updateEvent.release()
        
    def requestObjUpdate(self, obj):
        logging.debug("Requesting %s" % obj)
        self.uavTalk.sendObjReq(obj)
        
    def waitObjUpdate(self, obj, request=True, timeout=.5):
        logging.debug("Waiting for %s " % obj)
        cnt = obj.updateCnt
        if request:
            self.requestObjUpdate(obj)
        obj.updateEvent.acquire()
        obj.updateEvent.wait(timeout)
        obj.updateEvent.release()
        timeout = (cnt == obj.updateCnt)
        logging.debug("-> Waiting for %s Done. " % (obj))
        if timeout:
            s = "Timeout waiting for %s" % obj
            logging.debug(s)
            raise TimeoutException(s)
        
    def objLocallyUpdated(self, obj):
        # TODO: should check meta-data what to do
        self.uavTalk.sendObject(obj)
        
    def requestAllObjUpdate(self):
        for objId, obj in self.objs.items():
            if not obj.isMetaData():
                #print "GetMeta %s" % obj
                try:
                    logging.debug("Getting %s" % obj)
                    self.waitObjUpdate(obj, request=True, timeout=.1)
                    logging.debug("  Getting %s" % obj.metadata)
                    self.waitObjUpdate(obj.metadata, request=True, timeout=.1)
                except TimeoutException:
                    logging.debug("  TIMEOUT")
                    pass
                    
    def disableAllAutomaticUpdates(self):

        objsToExclude = [self.getObjByName("GCSTelemetryStats"), self.getObjByName("FlightTelemetryStats"), self.getObjByName("ObjectPersistence")]
        for i in xrange(len(objsToExclude)):
            objsToExclude[i] = objsToExclude[i].metadata.objId
            
        for objId, obj in self.objs.items():
            if obj.isMetaData() and obj.updateCnt>0:
                if obj.objId not in objsToExclude:
                    #print "Disabling automatic updates for %s" % (obj)
                    #print obj.telemetryUpdateMode.value
                    obj.telemetryUpdateMode.value = UAVMetaDataObject.UpdateMode.MANUAL
                    self.uavTalk.sendObject(obj)
                    
