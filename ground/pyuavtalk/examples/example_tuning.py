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
    


def printUsage():
    appName = os.path.basename(sys.argv[0])
    print
    print "usage:"
    print "  %s port " % appName
    print "  for example: %s COM30 " % appName
    print 
    
if __name__ == '__main__':
    try:
        if len(sys.argv) != 2:
          print "ERROR: Incorrect number of arguments"
          printUsage()
          sys.exit(2)
        
        port = sys.argv[1]
    
        if port[:3].upper() == "COM":
            _port = int(port[3:])-1
        else:
            _port = port
            
        serPort = serial.Serial(_port, 57600, timeout=.5)
        uavTalk = UavTalk(serPort)
        objMan = ObjManager(uavTalk)
        objMan.importDefinitions()
        uavTalk.start()
        
        import objectpersistence

        print "Getting Current Settings:"        
        while True:
            try:
                time.sleep(.5)
                objMan.StabilizationSettings.getUpdate()
                break
            except TimeoutException:
                print "Timeout"

        while True:
            while True:        
                print
                print
                print "q. Quit"
                print "s. Save settings"
                print
                print "1. Tune  Roll  Rate      %2.4f %2.4f %2.4f" % tuple(objMan.StabilizationSettings.RollRatePI.value)
                print "2. Tune  Pitch Rate      %2.4f %2.4f %2.4f" % tuple(objMan.StabilizationSettings.PitchRatePI.value)
                print "3. Tune  Yaw   Rate      %2.4f %2.4f %2.4f" % tuple(objMan.StabilizationSettings.YawRatePI.value)
                print
                print "4. Tune  Roll  Attitude  %2.4f %2.4f %2.4f" % tuple(objMan.StabilizationSettings.RollPI.value)
                print "5. Tune  Pitch Attitude  %2.4f %2.4f %2.4f" % tuple(objMan.StabilizationSettings.PitchPI.value)
                print "6. Tune  Yaw   Attitude  %2.4f %2.4f %2.4f" % tuple(objMan.StabilizationSettings.YawPI.value)
                print
                sel = raw_input()
                if len(sel) != 1:
                    continue
                elif sel == "q":
                    exit(0)
                elif sel == "s":
                    print "Saving ",
                    objMan.ObjectPersistence.Operation.value = objectpersistence.OperationField.SAVE
                    objMan.ObjectPersistence.Selection.value = objectpersistence.SelectionField.SINGLEOBJECT
                    objMan.ObjectPersistence.ObjectID.value = objMan.StabilizationSettings.objId
                    objMan.ObjectPersistence.InstanceID.value = objMan.StabilizationSettings.instId
                    objMan.ObjectPersistence.updated()
                    for i in range(10):
                        print ".",
                        objMan.ObjectPersistence.getUpdate(timeout=1)
                        if objMan.ObjectPersistence.Operation.value == objectpersistence.OperationField.COMPLETED:
                            print "Done"
                            break
                    print        
                elif sel == "1":
                    PI = objMan.StabilizationSettings.RollRatePI.value
                    break
                elif sel == "2":
                    PI = objMan.StabilizationSettings.PitchRatePI.value
                    break
                elif sel == "3":
                    PI = objMan.StabilizationSettings.YawRatePI.value
                    break
                elif sel == "4":
                    PI = objMan.StabilizationSettings.RollPI.value
                    break
                elif sel == "5":
                    PI = objMan.StabilizationSettings.PitchPI.value
                    break
                elif sel == "6":
                    PI = objMan.StabilizationSettings.YawPI.value
                    break
            
            while True:        
                print
                print 
                print "q. Quit"
                print
                print "1. tune K       %2.4f" % PI[0]
                print "2. tune I       %2.4f" % PI[1]
                print "3. tune I Limit %2.4f" % PI[2]
                sel = raw_input()
                if len(sel) != 1:
                    continue
                elif sel == "q":
                    exit(0)
                elif sel == "1":
                    PIIndex = 0
                    break
                elif sel == "2":
                    PIIndex = 1
                    break
                elif sel == "3":
                    PIIndex = 2
                    break
            
            print
            
            while True:
                try:
                    print "Current value: %2.4f" % PI[PIIndex]        
                    print "Tune-range from",
                    tuneFrom = float(raw_input())
                    print "Tune-range to",
                    tuneTo = float(raw_input())
                    break
                except Exception:
                    pass
                                
                                
            print
            print     
            while True:
                try:
                    # get update of ManualControlCommand 
                    objMan.ManualControlCommand.getUpdate(timeout=.5)  
                    
                    # calculate value out of Accessory1 input (-1 ... +1)
                    txControl = objMan.ManualControlCommand.Accessory1.value
                    value = tuneFrom + ((txControl+1)/2)*(tuneTo-tuneFrom)
                    PI[PIIndex] = value
                    objMan.StabilizationSettings.updated()
                    
                    print "\r%-1.2f => %2.4f" % (txControl, value),
                    time.sleep(.1)
                    
                except TimeoutException:
                    print "Timeout \a"
                except KeyboardInterrupt:
                    break
            
            print
            
    
    except Exception,e:
        print
        print "An error occured: ", e
        print
        traceback.print_exc()
    
    try:
        print "Stop"
        uavTalk.stop()
    except Exception:
        pass
    
    raw_input("Press ENTER, the application will close")


