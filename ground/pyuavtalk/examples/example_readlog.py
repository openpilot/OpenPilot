##
##############################################################################
#
# @file       example_readlog.py
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
import serial
import traceback
import sys

from openpilot.uavtalk.uavobject import *
from openpilot.uavtalk.uavtalk import *
from openpilot.uavtalk.objectManager import *
from openpilot.uavtalk.connectionManager import *
    
    
def _hex02(value):
    return "%02X" % value
    
class UavtalkDemo():
    
    def __init__(self):
        self.nbUpdates = 0
        self.lastRateCalc = time.time()
        self.updateRate = 0
        self.objMan = None
        self.connMan = None
        
    def setup(self, port, filename):
        print "Opening File \"%s\"" % filename
        file = open(filename,"rb")
        if file == None:
            raise IOError("Failed to open file")
        
        print "Creating UavTalk"
        self.uavTalk = UavTalk(None, filename)
        
        print "Starting ObjectManager"
        self.objMan = ObjManager(self.uavTalk)
        self.objMan.importDefinitions()
        
        print "Starting UavTalk"
        self.uavTalk.start()
        
    def stop(self):
        if self.uavTalk:
            print "Stopping UavTalk"
            self.uavTalk.stop()
        
    def showAttitudeViaObserver(self):
        print "Request fast periodic updates for AttitudeActual"
        self.objMan.AttitudeActual.metadata.telemetryUpdateMode.value = UAVMetaDataObject.UpdateMode.PERIODIC
        self.objMan.AttitudeActual.metadata.telemetryUpdatePeriod.value = 50
        self.objMan.AttitudeActual.metadata.updated()
        
        print "Install Observer for AttitudeActual updates\n"
        self.objMan.regObjectObserver(self.objMan.AttitudeActual, self, "_onAttitudeUpdate")
        # Spin until we get interrupted
        while True:
            time.sleep(1)
        
    def showAttitudeViaWait(self):
        print "Request fast periodic updates for AttitudeActual"
        self.objMan.AttitudeActual.metadata.telemetryUpdateMode.value = UAVMetaDataObject.UpdateMode.PERIODIC
        self.objMan.AttitudeActual.metadata.telemetryUpdatePeriod.value = 50
        self.objMan.AttitudeActual.metadata.updated()
        
        while True:
            self.objMan.AttitudeActual.waitUpdate()
            self._onAttitudeUpdate(self.objMan.AttitudeActual)
                    
    def showAttitudeViaGet(self):
        while True:
            self.objMan.AttitudeActual.getUpdate()
            self._onAttitudeUpdate(self.objMan.AttitudeActual)
        
    def _onAttitudeUpdate(self, args):
        self.nbUpdates += 1
        
        now = time.time()    
        if now-self.lastRateCalc > 1:
            self.updateRate = self.nbUpdates/(now-self.lastRateCalc) 
            self.lastRateCalc = now
            self.nbUpdates = 0
            
        if self.nbUpdates & 1: 
            dot = "." 
        else: 
            dot= " "
        
        print " %s Rate: %02.1f Hz  " % (dot, self.updateRate),
            
        roll = self.objMan.AttitudeActual.Roll.value
        print "Roll: %-4d " % roll,
        i = roll/90
        if i<-1: i=-1
        if i>1: i= 1
        i = int((i+1)*15)
        print "-"*i+"*"+"-"*(30-i)+" \r",
        
    def driveServo(self):
        print "Taking control of self.actuatorCmd"
        self.objMan.ActuatorCommand.metadata.access.value = UAVMetaDataObject.Access.READONLY
        self.objMan.ActuatorCommand.metadata.updated()   
        self.objMan.ManualControlCommand.metadata.access.value = UAVMetaDataObject.Access.READONLY
        self.objMan.ManualControlCommand.metadata.updated()
        
        while True:
            self.objMan.ActuatorCommand.Channel.value[0] = 1000
            self.objMan.ActuatorCommand.updated()
            time.sleep(1)
            
            self.objMan.ActuatorCommand.Channel.value[0] = 2000
            self.objMan.ActuatorCommand.updated()
            time.sleep(1)
            

def printUsage():
    appName = os.path.basename(sys.argv[0])
    print
    print "usage:"
    print "  %s filename " % appName
    print
    print "  for example: %s /tmp/OP-2015-04-28_23-16-33.opl" % appName
    print 
    
if __name__ == '__main__':
    
    if len(sys.argv) !=2:
         print "ERROR: Incorrect number of arguments"
	 print len(sys.argv)
         printUsage()
         sys.exit(2)

    script, filename = sys.argv
    if not os.path.exists(sys.argv[1]):
    	sys.exit('ERROR: Database %s was not found!' % sys.argv[1])

    # Log everything, and send it to stderr.
    logging.basicConfig(level=logging.INFO)

    try:
        demo = UavtalkDemo()
        demo.setup(None, filename)


            
    except KeyboardInterrupt:
        pass
    except Exception,e:
        print
        print "An error occured: ", e
        print
        traceback.print_exc()
    
    print
    


