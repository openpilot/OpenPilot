##
##############################################################################
#
# @file       uavtalk.py
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


import time
import logging
import threading
        
SYNC = 0x3C
VERSION_MASK = 0xFC
VERSION = 0x20
TYPE_MASK  = 0x03
TYPE_OBJ = 0x00
TYPE_OBJ_REQ = 0x01
TYPE_OBJ_ACK = 0x02
TYPE_ACK = 0x03
TYPE_NACK = 0x04

MIN_HEADER_LENGTH = 10  # sync(1), type (1), size(2), object ID(4)
MAX_HEADER_LENGTH = 10 # sync(1), type (1), size(2), object ID (4), instance ID(2 not used in single objects)

MAX_PAYLOAD_LENGTH = 255
CHECKSUM_LENGTH = 1
MAX_PACKET_LENGTH = (MAX_HEADER_LENGTH + MAX_PAYLOAD_LENGTH + CHECKSUM_LENGTH)
    

        

class Crc(object):
    
    crcTable = ( 0x00, 0x07, 0x0e, 0x09, 0x1c,
            0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
            0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46,
            0x41, 0x54, 0x53, 0x5a, 0x5d, 0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb,
            0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd, 0x90,
            0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1,
            0xb4, 0xb3, 0xba, 0xbd, 0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5,
            0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea, 0xb7, 0xb0,
            0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93,
            0x94, 0x9d, 0x9a, 0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32,
            0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a, 0x57, 0x50, 0x59,
            0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74,
            0x7d, 0x7a, 0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1,
            0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4, 0xf9, 0xfe, 0xf7, 0xf0,
            0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3,
            0xd4, 0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56,
            0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44, 0x19, 0x1e, 0x17, 0x10, 0x05,
            0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
            0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78,
            0x7f, 0x6a, 0x6d, 0x64, 0x63, 0x3e, 0x39, 0x30, 0x37, 0x22, 0x25,
            0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, 0xae,
            0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f,
            0x8a, 0x8d, 0x84, 0x83, 0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc,
            0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3 )
    
    def __init__(self):
        self.reset()
        
    def reset(self, firstValue=None):
        self.crc = 0
        if firstValue != None:
            self.add(firstValue)
        
    def read(self):
        return self.crc
    
    def add(self, value):
        try:
                self.crc = Crc.crcTable[self.crc ^ (value & 0xff)]
        except TypeError:
                print "Likely End Of File"
                raise SystemExit
        
    def addList(self, values):
        for v in values:
            self.add(v)
        
        
class UavTalkRecThread(threading.Thread):
    STATE_SYNC = 0
    STATE_TYPE = 1 
    STATE_SIZE = 2
    STATE_OBJID = 3
    STATE_INSTID = 4
    STATE_DATA = 5
    STATE_CS = 6
    
    def __init__(self, uavTalk):
        threading.Thread.__init__(self)
        self.uavTalk = uavTalk
        self.rxState = self.STATE_SYNC
        self.rxCrc = Crc()
        self.stop = False
        
    def run(self):
        #self.uavTalk.serial.open()
        self.stop = False

        if self.uavTalk.logFile is not None:
            file = open(self.uavTalk.logFile)
            while not self.stop:
                rx = file.read(1)
                if len(rx) > 0:
                    rx = ord(rx)
                self._consumeByte(rx)
        elif self.uavTalk.serial is not None:
            while not self.stop:
                rx = self.uavTalk.serial.read(1)
                if len(rx) > 0:
                    rx = ord(rx)
    #                if (rx == SYNC):
    #                    print
    #                print hex(rx),
                    self._consumeByte(rx)
        else:
            print "Nothing to do!"

                
    def _consumeByte(self, rx):
        self.rxCrc.add(rx)        
        
        if self.rxState == UavTalkRecThread.STATE_SYNC:
            if rx == SYNC:
                self.rxCrc.reset(rx)       
                self.rxState += 1
#            else:
#                logging.warning("NoSync")
                
        elif self.rxState == UavTalkRecThread.STATE_TYPE:
            if (rx & VERSION_MASK != VERSION):
                self.rxState == UavTalkRecThread.STATE_SYNC
            else:
                self.rxType = rx & TYPE_MASK
                self.rxCount = 0
                self.rxSize = 0
                self.rxState += 1
                
        elif self.rxState == UavTalkRecThread.STATE_SIZE:
            self.rxSize >>= 8
            self.rxSize |= (rx<<8)
            self.rxCount += 1
            
            if self.rxCount == 2:    
                # Received complete packet size, check for valid packet size
                if (self.rxSize < MIN_HEADER_LENGTH) or (self.rxSize > MAX_HEADER_LENGTH + MAX_PAYLOAD_LENGTH):
                    logging.error("INVALID Packet Size")
                    self.rxState = UavTalkRecThread.STATE_SYNC
                else:
                    self.rxCount = 0
                    self.rxObjId = 0
                    self.rxState = UavTalkRecThread.STATE_OBJID
                    
        elif self.rxState == UavTalkRecThread.STATE_OBJID:
            self.rxObjId >>= 8
            self.rxObjId |= (rx<<24)
            self.rxCount += 1

            if self.rxCount < 4:
                return

            self.rxCount = 0
            self.rxInstId = 0
            self.rxState += 1

        elif self.rxState == UavTalkRecThread.STATE_INSTID:
            self.rxCount += 1

            if self.rxCount < 2:
                return

            self.rxCount = 0

            # Received complete ObjID
            self.obj = self.uavTalk.objMan.getObj(self.rxObjId)
            if self.obj != None:
                self.rxDataSize = self.obj.getSerialisedSize()

                if MIN_HEADER_LENGTH + self.obj.getSerialisedSize() != self.rxSize:
                    logging.error("packet Size MISMATCH")
                    self.rxState = UavTalkRecThread.STATE_SYNC
                else:
                    print "Object: %s" % self.obj
                    self.rxCount = 0
                    self.rxData = []

                    if self.rxType == TYPE_OBJ_REQ | self.rxType == TYPE_ACK | self.rxType == TYPE_NACK:
                        self.rxDataSize = 0

                    if self.rxDataSize > 0:
                        self.rxState = UavTalkRecThread.STATE_DATA
                    else:
                        self.rxState = UavTalkRecThread.STATE_DATA+1
            else:
                logging.warning("Rec UNKNOWN Obj %x", self.rxObjId)
                self.rxState = self.STATE_SYNC


        elif self.rxState == UavTalkRecThread.STATE_DATA:
            self.rxData.append(rx)
            self.rxCount += 1
            if self.rxCount == self.rxDataSize:
                self.rxState += 1
            #else:
            #    logging.error("Obj %x INVALID SIZE", self.rxObjId)
                
        elif self.rxState == UavTalkRecThread.STATE_CS:
            # by now, the CS has been added to the CRC calc, so now the CRC calc should read 0
            if self.rxCrc.read() != 0:
                logging.error("CRC ERROR")
            else:
                self.uavTalk._onRecevedPacket(self.obj, self.rxType, self.rxData)    
                self.rxState = UavTalkRecThread.STATE_SYNC
            
        else:
            logging.error("INVALID STATE")
            self.rxState = UavTalkRecThread.STATE_SYNC


class UavTalk(object):

    def __init__(self, serial, logFile):
        self.logFile = logFile
        self.serial = serial
        self.objMan = None
        self.txLock = threading.Lock()
        
    def setObjMan(self, objMan):
        self.objMan = objMan
        
    def start(self):
        self.recThread = UavTalkRecThread(self)
        self.recThread.start()
        
    def stop(self):
        self.recThread.stop = True;
        self.recThread.join()
        
    def _onRecevedPacket(self, obj, rxType, rxData): 
        logging.debug("REC Obj %20s type %x cnt %d", obj, rxType, obj.updateCnt+1)
#                for i in rxData: print hex(i),
#                print
               
        if rxType == TYPE_OBJ_ACK:
            logging.debug("Sending ACK for Obj %s", obj)
            self.sendObjectAck(obj)
            
        self.objMan.objUpdate(obj, rxData)
         
    def sendObjReq(self, obj):
        self._sendpacket(TYPE_OBJ_REQ, obj.objId)
        
    def sendObjectAck(self, obj):
        self._sendpacket(TYPE_ACK, obj.objId)
        
    def sendObject(self, obj, reqAck=False):
        if reqAck:
            type = TYPE_OBJ_ACK
        else:
            type = TYPE_OBJ
        self._sendpacket(type, obj.objId, obj.serialize())
                
    def _sendpacket(self, type, objId, data=None):
        
        self.txLock.acquire()
        
        header = [SYNC, type | VERSION, 0, 0, 0, 0, 0, 0]
        
        length = MIN_HEADER_LENGTH 
        if data != None: 
            length += len(data)
        header[2] = length & 0xFF
        header[3] = (length >>8) & 0xFF
        for i in xrange(4,8): 
            header[i] = objId & 0xff
            objId >>= 8
        
        crc = Crc()
        crc.addList(header)
        self.serial.write("".join(map(chr,header)))
        
        if data != None:
            crc.addList(data)
            self.serial.write("".join(map(chr,data)))
        
        self.serial.write(chr(crc.read()))
        
        self.txLock.release()
        

