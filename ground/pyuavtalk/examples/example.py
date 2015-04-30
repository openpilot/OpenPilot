##
##############################################################################
#
# @file       example.py
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
        
    def setup(self, port):
        print "Opening Port \"%s\"" % port
        if port[:3].upper() == "COM":
            _port = int(port[3:])-1
        else:
            _port = port
        serPort = serial.Serial(_port, 57600, timeout=.5)
        if not serPort.isOpen():
            raise IOError("Failed to open serial port")
        
        print "Creating UavTalk"
        self.uavTalk = UavTalk(serPort)
        
        print "Starting ObjectManager"
        self.objMan = ObjManager(self.uavTalk)
        self.objMan.importDefinitions()
        
        print "Starting UavTalk"
        self.uavTalk.start()
        
        print "Starting ConnectionManager"
        self.connMan = ConnectionManager(self.uavTalk, self.objMan)
        
        print "Connecting...",
        self.connMan.connect()
        print "Connected"
        
        print "Getting all Data"
        self.objMan.requestAllObjUpdate()
        
        print "SN:",
        sn = self.objMan.FirmwareIAPObj.CPUSerial.value
        print "".join(map(_hex02, sn))
        
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
    print "  %s port o|w|g|s" % appName
    print "  o: Show Attitude using an \"observer\""
    print "  w: Show Attitude waiting for updates from flight"
    print "  g: Show Attitude performing get operations"
    print "  s: Drive Servo"
    print
    print "  for example: %s COM30 o" % appName
    print 
    
if __name__ == '__main__':
    
    if len(sys.argv) != 3:
        print "ERROR: Incorrect number of arguments"
        printUsage()
        sys.exit(2)
        
    port, option = sys.argv[1:]

    if option not in ["o","w","g","s"]:
        print "ERROR: Invalid option"
        printUsage()
        sys.exit(2)

    # Log everything, and send it to stderr.
    logging.basicConfig(level=logging.INFO)

    try:
        demo = UavtalkDemo()
        demo.setup(port)

        if option == "o":        
            demo.showAttitudeViaObserver()      # will not return
        elif option == "w":        
            demo.showAttitudeViaWait()          # will not return
        if option == "g":        
            demo.showAttitudeViaGet()           # will not return
        if option == "s":        
            demo.driveServo()                   # will not return
            
    except KeyboardInterrupt:
        pass
    except Exception,e:
        print
        print "An error occured: ", e
        print
        traceback.print_exc()
    
    print
    
    try:
        demo.stop()
    except Exception:
        pass
    
    raw_input("Press ENTER, the application will close")


