/**
 ******************************************************************************
 *
 * @file       uavobjectutilmanager.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectUtilPlugin UAVObjectUtil Plugin
 * @{
 * @brief      The UAVUObjectUtil GCS plugin
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "uavobjectutilmanager.h"

#include "utils/homelocationutil.h"

#include <QMutexLocker>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include <objectpersistence.h>

#include "firmwareiapobj.h"
#include "homelocation.h"
#include "gpsposition.h"

// ******************************
// constructor/destructor

UAVObjectUtilManager::UAVObjectUtilManager()
{
    mutex = new QMutex(QMutex::Recursive);
    saveState = IDLE;
    failureTimer.stop();
    failureTimer.setSingleShot(true);
    failureTimer.setInterval(1000);
    connect(&failureTimer, SIGNAL(timeout()),this,SLOT(objectPersistenceOperationFailed()));

    pm = NULL;
    obm = NULL;
    obum = NULL;

    pm = ExtensionSystem::PluginManager::instance();
    if (pm)
    {
        obm = pm->getObject<UAVObjectManager>();
        obum = pm->getObject<UAVObjectUtilManager>();
    }

}

UAVObjectUtilManager::~UAVObjectUtilManager()
{
//	while (!queue.isEmpty())
	{
	}

	disconnect();

	if (mutex)
	{
		delete mutex;
		mutex = NULL;
	}
}


UAVObjectManager* UAVObjectUtilManager::getObjectManager() {
    Q_ASSERT(obm);
    return obm;
}


// ******************************
// SD card saving
//

/*
  Add a new object to save in the queue
  */
void UAVObjectUtilManager::saveObjectToSD(UAVObject *obj)
{
    // Add to queue
    queue.enqueue(obj);
    qDebug() << "Enqueue object: " << obj->getName();


    // If queue length is one, then start sending (call sendNextObject)
    // Otherwise, do nothing, it's sending anyway
    if (queue.length()==1)
        saveNextObject();


}

void UAVObjectUtilManager::saveNextObject()
{
    if ( queue.isEmpty() )
    {
        return;
    }

    Q_ASSERT(saveState == IDLE);

    // Get next object from the queue
    UAVObject* obj = queue.head();
    qDebug() << "Send save object request to board " << obj->getName();

    ObjectPersistence* objper = dynamic_cast<ObjectPersistence*>( getObjectManager()->getObject(ObjectPersistence::NAME) );
    connect(objper, SIGNAL(transactionCompleted(UAVObject*,bool)), this, SLOT(objectPersistenceTransactionCompleted(UAVObject*,bool)));
    connect(objper, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(objectPersistenceUpdated(UAVObject *)));
    saveState = AWAITING_ACK;
    if (obj != NULL)
    {
        ObjectPersistence::DataFields data;
        data.Operation = ObjectPersistence::OPERATION_SAVE;
        data.Selection = ObjectPersistence::SELECTION_SINGLEOBJECT;
        data.ObjectID = obj->getObjID();
        data.InstanceID = obj->getInstID();
        objper->setData(data);
        objper->updated();
    }
    // Now: we are going to get two "objectUpdated" messages (one coming from GCS, one coming from Flight, which
    // will confirm the object was properly received by both sides) and then one "transactionCompleted" indicating
    // that the Flight side did not only receive the object but it did receive it without error. Last we will get
    // a last "objectUpdated" message coming from flight side, where we'll get the results of the objectPersistence
    // operation we asked for (saved, other).
}

/**
  * @brief Process the transactionCompleted message from Telemetry indicating request sent successfully
  * @param[in] The object just transsacted.  Must be ObjectPersistance
  * @param[in] success Indicates that the transaction did not time out
  *
  * After a failed transaction (usually timeout) resends the save request.  After a succesful
  * transaction will then wait for a save completed update from the autopilot.
  */
void UAVObjectUtilManager::objectPersistenceTransactionCompleted(UAVObject* obj, bool success)
{
    if(success) {
        Q_ASSERT(obj->getName().compare("ObjectPersistence") == 0);
        Q_ASSERT(saveState == AWAITING_ACK);
        // Two things can happen:
        // Either the Object Save Request did actually go through, and then we should get in
        // "AWAITING_COMPLETED" mode, or the Object Save Request did _not_ go through, for example
        // because the object does not exist and then we will never get a subsequent update.
        // For this reason, we will arm a 1 second timer to make provision for this and not block
        // the queue:
        saveState = AWAITING_COMPLETED;
        disconnect(obj, SIGNAL(transactionCompleted(UAVObject*,bool)), this, SLOT(objectPersistenceTransactionCompleted(UAVObject*,bool)));
        failureTimer.start(2000); // Create a timeout
    } else {
        // Can be caused by timeout errors on sending.  Forget it and send next.
        qDebug() << "objectPersistenceTranscationCompleted (error)";
        UAVObject *obj = getObjectManager()->getObject(ObjectPersistence::NAME);
        obj->disconnect(this);
        queue.dequeue(); // We can now remove the object, it failed anyway.
        saveState = IDLE;
        emit saveCompleted(obj->getField("ObjectID")->getValue().toInt(), false);
        saveNextObject();
    }
}

/**
  * @brief Object persistence operation failed, i.e. we never got an update
  * from the board saying "completed".
  */
void UAVObjectUtilManager::objectPersistenceOperationFailed()
{
    if(saveState == AWAITING_COMPLETED) {
        //TODO: some warning that this operation failed somehow
        // We have to disconnect the object persistence 'updated' signal
        // and ask to save the next object:

        ObjectPersistence * objectPersistence = ObjectPersistence::GetInstance(getObjectManager());
        Q_ASSERT(objectPersistence);

        UAVObject* obj = queue.dequeue(); // We can now remove the object, it failed anyway.
        Q_ASSERT(obj);

        objectPersistence->disconnect(this);

        saveState = IDLE;
        emit saveCompleted(obj->getObjID(), false);

        saveNextObject();
    }

}



/**
  * @brief Process the ObjectPersistence updated message to confirm the right object saved
  * then requests next object be saved.
  * @param[in] The object just received.  Must be ObjectPersistance
  */
void UAVObjectUtilManager::objectPersistenceUpdated(UAVObject * obj)
{
    Q_ASSERT(obj);
    Q_ASSERT(obj->getObjID() == ObjectPersistence::OBJID);
    ObjectPersistence::DataFields objectPersistence = ((ObjectPersistence *)obj)->getData();

    if(saveState == AWAITING_COMPLETED && objectPersistence.Operation == ObjectPersistence::OPERATION_ERROR) {
        failureTimer.stop();
        objectPersistenceOperationFailed();
    } else if (saveState == AWAITING_COMPLETED &&
               objectPersistence.Operation == ObjectPersistence::OPERATION_COMPLETED) {
        failureTimer.stop();
        // Check right object saved
        UAVObject* savingObj = queue.head();
        if(objectPersistence.ObjectID != savingObj->getObjID() ) {
            objectPersistenceOperationFailed();
            return;
        }

        obj->disconnect(this);
        queue.dequeue(); // We can now remove the object, it's done.
        saveState = IDLE;

        emit saveCompleted(objectPersistence.ObjectID, true);
        saveNextObject();
    }
}

/**
  * Helper function that makes sure FirmwareIAP is updated and then returns the data
  */
FirmwareIAPObj::DataFields UAVObjectUtilManager::getFirmwareIap()
{
    FirmwareIAPObj::DataFields dummy;

    FirmwareIAPObj *firmwareIap = FirmwareIAPObj::GetInstance(obm);
    Q_ASSERT(firmwareIap);
    if (!firmwareIap)
        return dummy;

    return firmwareIap->getData();
}

/**
  * Get the UAV Board model, for anyone interested. Return format is:
  * (Board Type << 8) + BoardRevision.
  */
int UAVObjectUtilManager::getBoardModel()
{
    FirmwareIAPObj::DataFields firmwareIapData = getFirmwareIap();
    qDebug()<<"Board type="<<firmwareIapData.BoardType;
    qDebug()<<"Board revision="<<firmwareIapData.BoardRevision;
    int ret=firmwareIapData.BoardType <<8;
    ret = ret + firmwareIapData.BoardRevision;
    qDebug()<<"Board info="<<ret;
    return ret;
}

/**
  * Get the UAV Board CPU Serial Number, for anyone interested. Return format is a byte array
  */
QByteArray UAVObjectUtilManager::getBoardCPUSerial()
{
    QByteArray cpuSerial;
    FirmwareIAPObj::DataFields firmwareIapData = getFirmwareIap();

    for (unsigned int i = 0; i < FirmwareIAPObj::CPUSERIAL_NUMELEM; i++)
        cpuSerial.append(firmwareIapData.CPUSerial[i]);

    return cpuSerial;
}

quint32 UAVObjectUtilManager::getFirmwareCRC()
{
    FirmwareIAPObj::DataFields firmwareIapData = getFirmwareIap();
    return firmwareIapData.crc;
}

/**
  * Get the UAV Board Description, for anyone interested.
  */
QByteArray UAVObjectUtilManager::getBoardDescription()
{
    QByteArray ret;
    FirmwareIAPObj::DataFields firmwareIapData = getFirmwareIap();

    for (unsigned int i = 0; i < FirmwareIAPObj::DESCRIPTION_NUMELEM; i++)
        ret.append(firmwareIapData.Description[i]);

    return ret;
}



// ******************************
// HomeLocation

int UAVObjectUtilManager::setHomeLocation(double LLA[3], bool save_to_sdcard)
{
    double Be[3];

    int result = Utils::HomeLocationUtil().getDetails(LLA, Be);
    Q_ASSERT(result == 0);

    // save the new settings
    HomeLocation *homeLocation = HomeLocation::GetInstance(obm);
    Q_ASSERT(homeLocation != NULL);

    HomeLocation::DataFields homeLocationData = homeLocation->getData();
    homeLocationData.Latitude = LLA[0] * 1e7;
    homeLocationData.Longitude = LLA[1] * 1e7;
    homeLocationData.Altitude = LLA[2];

    homeLocationData.Be[0] = Be[0];
    homeLocationData.Be[1] = Be[1];
    homeLocationData.Be[2] = Be[2];

    homeLocationData.Set = HomeLocation::SET_TRUE;

    homeLocation->setData(homeLocationData);

    if (save_to_sdcard)
        saveObjectToSD(homeLocation);

    return 0;
}

int UAVObjectUtilManager::getHomeLocation(bool &set, double LLA[3])
{
    HomeLocation *homeLocation = HomeLocation::GetInstance(obm);
    Q_ASSERT(homeLocation != NULL);

    HomeLocation::DataFields homeLocationData = homeLocation->getData();

    set = homeLocationData.Set;

    LLA[0] = homeLocationData.Latitude*1e-7;
    LLA[1] = homeLocationData.Longitude*1e-7;
    LLA[2] = homeLocationData.Altitude;

	if (LLA[0] != LLA[0]) LLA[0] = 0; // nan detection
	else
	if (LLA[0] >  90) LLA[0] =  90;
	else
	if (LLA[0] < -90) LLA[0] = -90;

	if (LLA[1] != LLA[1]) LLA[1] = 0; // nan detection
	else
	if (LLA[1] >  180) LLA[1] =  180;
	else
	if (LLA[1] < -180) LLA[1] = -180;

	if (LLA[2] != LLA[2]) LLA[2] = 0; // nan detection

	return 0;	// OK
}


// ******************************
// GPS

int UAVObjectUtilManager::getGPSPosition(double LLA[3])
{
    GPSPosition *gpsPosition = GPSPosition::GetInstance(obm);
    Q_ASSERT(gpsPosition != NULL);

    GPSPosition::DataFields gpsPositionData = gpsPosition->getData();

    LLA[0] = gpsPositionData.Latitude;
    LLA[1] = gpsPositionData.Longitude;
    LLA[2] = gpsPositionData.Altitude;

	if (LLA[0] != LLA[0]) LLA[0] = 0; // nan detection
	else
	if (LLA[0] >  90) LLA[0] =  90;
	else
	if (LLA[0] < -90) LLA[0] = -90;

	if (LLA[1] != LLA[1]) LLA[1] = 0; // nan detection
	else
	if (LLA[1] >  180) LLA[1] =  180;
	else
	if (LLA[1] < -180) LLA[1] = -180;

	if (LLA[2] != LLA[2]) LLA[2] = 0; // nan detection

	return 0;	// OK
}

deviceDescriptorStruct UAVObjectUtilManager::getBoardDescriptionStruct()
{
    deviceDescriptorStruct ret;
    descriptionToStructure(getBoardDescription(),ret);
    return ret;
}

bool UAVObjectUtilManager::descriptionToStructure(QByteArray desc, deviceDescriptorStruct & struc)
{
   if (desc.startsWith("OpFw")) {
       /*
        * This looks like a binary with a description at the end
        *  4 bytes: header: "OpFw"
        *  4 bytes: git commit hash (short version of SHA1)
        *  4 bytes: Unix timestamp of last git commit
        *  2 bytes: target platform. Should follow same rule as BOARD_TYPE and BOARD_REVISION in board define files.
        *  26 bytes: commit tag if it is there, otherwise "Unreleased". Zero-padded
        *  20 bytes: SHA1 sum of the firmware.
        *  20 bytes: SHA1 sum of the UAVO definition files.
        *  20 bytes: free for now.
        */

       // Note: the ARM binary is big-endian:
       quint32 gitCommitHash = desc.at(7) & 0xFF;
       for (int i = 1; i < 4; i++) {
           gitCommitHash = gitCommitHash << 8;
           gitCommitHash += desc.at(7-i) & 0xFF;
       }
       struc.gitHash = QString("%1").arg(gitCommitHash, 8, 16, QChar('0'));

       quint32 gitDate = desc.at(11) & 0xFF;
       for (int i = 1; i < 4; i++) {
           gitDate = gitDate << 8;
           gitDate += desc.at(11-i) & 0xFF;
       }
       struc.gitDate = QDateTime::fromTime_t(gitDate).toUTC().toString("yyyyMMdd HH:mm");

       QString gitTag = QString(desc.mid(14,26));
       struc.gitTag = gitTag;

       // TODO: check platform compatibility
       QByteArray targetPlatform = desc.mid(12,2);
       struc.boardType = (int)targetPlatform.at(0);
       struc.boardRevision = (int)targetPlatform.at(1);
       struc.fwHash.clear();
       struc.fwHash=desc.mid(40,20);
       struc.uavoHash.clear();
       struc.uavoHash=desc.mid(60,20);
       qDebug()<<__FUNCTION__<<":description from board:";
       foreach(char x,desc)
       {
           qDebug()<<QString::number(x,16);
       }

       qDebug()<<__FUNCTION__<<":uavoHash:";
       QByteArray array2=struc.uavoHash.data();
       foreach(char x,array2)
       {
           qDebug()<<QString::number(x,16);
       }
       return true;
   }
   return false;
}

// ******************************
