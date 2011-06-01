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

// ******************************
// constructor/destructor

UAVObjectUtilManager::UAVObjectUtilManager()
{
    mutex = new QMutex(QMutex::Recursive);
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

// ******************************
// SD card saving

void UAVObjectUtilManager::saveObjectToSD(UAVObject *obj)
{
	QMutexLocker locker(mutex);

	if (!obj) return;

	// Add to queue
	queue.enqueue(obj);

	// If queue length is one, then start sending (call sendNextObject)
	// Otherwise, do nothing, it's sending anyway
	if (queue.length() <= 1)
		saveNextObject();
}

void UAVObjectUtilManager::saveNextObject()
{
	if (queue.isEmpty()) return;

	// Get next object from the queue
	UAVObject *obj = queue.head();
	if (!obj) return;

	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	if (!pm) return;

	UAVObjectManager *om = pm->getObject<UAVObjectManager>();
	if (!om) return;

	ObjectPersistence *objper = dynamic_cast<ObjectPersistence *>(om->getObject(ObjectPersistence::NAME));
	connect(objper, SIGNAL(transactionCompleted(UAVObject *, bool)), this, SLOT(transactionCompleted(UAVObject *, bool)));

	ObjectPersistence::DataFields data;
	data.Operation = ObjectPersistence::OPERATION_SAVE;
	data.Selection = ObjectPersistence::SELECTION_SINGLEOBJECT;
	data.ObjectID = obj->getObjID();
	data.InstanceID = obj->getInstID();
	objper->setData(data);
	objper->updated();
}

void UAVObjectUtilManager::transactionCompleted(UAVObject *obj, bool success)
{
	Q_UNUSED(success);

	QMutexLocker locker(mutex);

	if (!obj) return;

	// Disconnect from sending object
	obj->disconnect(this);
	queue.dequeue();		// We can now remove the object, it's done.
	saveNextObject();
}

/**
  * Get the UAV Board model, for anyone interested. Return format is:
  * (Board Type << 8) + BoardRevision.
  */
int UAVObjectUtilManager::getBoardModel()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    if (!pm)
        return 0;
    UAVObjectManager *om = pm->getObject<UAVObjectManager>();
    if (!om)
        return 0;

    UAVDataObject *obj = dynamic_cast<UAVDataObject *>(om->getObject(QString("FirmwareIAPObj")));
    // The code below will ask for the object update and wait for the updated to be received,
    // or the timeout of the timer, set to 1 second.
    QEventLoop loop;
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), &loop, SLOT(quit()));
    QTimer::singleShot(1000, &loop, SLOT(quit())); // Create a timeout
    obj->requestUpdate();
    loop.exec();

    int boardType = (obj->getField("BoardType")->getValue().toInt()) << 8;
    boardType += obj->getField("BoardRevision")->getValue().toInt();
    return boardType;
}

/**
  * Get the UAV Board CPU Serial Number, for anyone interested. Return format is a byte array
  */
QByteArray UAVObjectUtilManager::getBoardCPUSerial()
{
    QByteArray cpuSerial;
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    if (!pm)
        return 0;
    UAVObjectManager *om = pm->getObject<UAVObjectManager>();
    if (!om)
        return 0;

    UAVDataObject *obj = dynamic_cast<UAVDataObject *>(om->getObject(QString("FirmwareIAPObj")));
    // The code below will ask for the object update and wait for the updated to be received,
    // or the timeout of the timer, set to 1 second.
    QEventLoop loop;
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), &loop, SLOT(quit()));
    QTimer::singleShot(1000, &loop, SLOT(quit())); // Create a timeout
    obj->requestUpdate();
    loop.exec();

    UAVObjectField* cpuField = obj->getField("CPUSerial");
    for (int i = 0; i < cpuField->getNumElements(); ++i) {
        cpuSerial.append(cpuField->getValue(i).toUInt());
    }
    return cpuSerial;
}

/**
  * Get the UAV Board Description, for anyone interested.
  */
QString UAVObjectUtilManager::getBoardDescription()
{
    QString description;
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    if (!pm)
        return 0;
    UAVObjectManager *om = pm->getObject<UAVObjectManager>();
    if (!om)
        return 0;

    UAVDataObject *obj = dynamic_cast<UAVDataObject *>(om->getObject(QString("FirmwareIAPObj")));
    // The code below will ask for the object update and wait for the updated to be received,
    // or the timeout of the timer, set to 1 second.
    QEventLoop loop;
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), &loop, SLOT(quit()));
    QTimer::singleShot(1000, &loop, SLOT(quit())); // Create a timeout
    obj->requestUpdate();
    loop.exec();

    UAVObjectField* descriptionField = obj->getField("Description");
    // Description starts with an offset of
    for (int i = 14; i < descriptionField->getNumElements(); ++i) {
        description.append(descriptionField->getValue(i).toChar());
    }
    return description;
}



// ******************************
// HomeLocation

int UAVObjectUtilManager::setHomeLocation(double LLA[3], bool save_to_sdcard)
{
	double ECEF[3];
	double RNE[9];
	double Be[3];
	UAVObjectField *field;

	QMutexLocker locker(mutex);

	if (Utils::HomeLocationUtil().getDetails(LLA, ECEF, RNE, Be) < 0)
		return -1;	// error

	// ******************
	// save the new settings

	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	if (!pm) return -2;

	UAVObjectManager *om = pm->getObject<UAVObjectManager>();
	if (!om) return -3;

	UAVDataObject *obj = dynamic_cast<UAVDataObject *>(om->getObject(QString("HomeLocation")));
	if (!obj) return -4;

	UAVObjectField *ECEF_field = obj->getField(QString("ECEF"));
	if (!ECEF_field) return -5;

	UAVObjectField *RNE_field = obj->getField(QString("RNE"));
	if (!RNE_field) return -6;

	UAVObjectField *Be_field = obj->getField(QString("Be"));
	if (!Be_field) return -7;

	field = obj->getField("Latitude");
	if (!field) return -8;
	field->setDouble(LLA[0] * 10e6);

	field = obj->getField("Longitude");
	if (!field) return -9;
	field->setDouble(LLA[1] * 10e6);

	field = obj->getField("Altitude");
	if (!field) return -10;
	field->setDouble(LLA[2]);

	for (int i = 0; i < 3; i++)
		ECEF_field->setDouble(ECEF[i] * 100, i);

	for (int i = 0; i < 9; i++)
		RNE_field->setDouble(RNE[i], i);

	for (int i = 0; i < 3; i++)
		Be_field->setDouble(Be[i], i);

	field = obj->getField("Set");
	if (!field) return -11;
	field->setValue("TRUE");

	obj->updated();

	// ******************
	// save the new setting to SD card

	if (save_to_sdcard)
		saveObjectToSD(obj);

	// ******************
	// debug
/*
	qDebug() << "setting HomeLocation UAV Object .. " << endl;
	QString s;
	s = "        LAT:" + QString::number(LLA[0], 'f', 7) + " LON:" + QString::number(LLA[1], 'f', 7) + " ALT:" + QString::number(LLA[2], 'f', 1);
	qDebug() << s << endl;
	s = "        ECEF "; for (int i = 0; i < 3; i++) s += " " + QString::number((int)(ECEF[i] * 100));
	qDebug() << s << endl;
	s = "        RNE  ";  for (int i = 0; i < 9; i++) s += " " + QString::number(RNE[i], 'f', 7);
	qDebug() << s << endl;
	s = "        Be   ";  for (int i = 0; i < 3; i++) s += " " + QString::number(Be[i], 'f', 2);
	qDebug() << s << endl;
*/
	// ******************

	return 0;	// OK
}

int UAVObjectUtilManager::getHomeLocation(bool &set, double LLA[3])
{
	UAVObjectField *field;

	QMutexLocker locker(mutex);

	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	if (!pm) return -1;

	UAVObjectManager *om = pm->getObject<UAVObjectManager>();
	if (!om) return -2;

	UAVDataObject *obj = dynamic_cast<UAVDataObject *>(om->getObject(QString("HomeLocation")));
	if (!obj) return -3;

//	obj->requestUpdate();

	field = obj->getField("Set");
	if (!field) return -4;
	set = field->getValue().toBool();

	field = obj->getField("Latitude");
	if (!field) return -5;
	LLA[0] = field->getDouble() * 1e-7;

	field = obj->getField("Longitude");
	if (!field) return -6;
	LLA[1] = field->getDouble() * 1e-7;

	field = obj->getField("Altitude");
	if (!field) return -7;
	LLA[2] = field->getDouble();

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

int UAVObjectUtilManager::getHomeLocation(bool &set, double LLA[3], double ECEF[3], double RNE[9], double Be[3])
{
	UAVObjectField *field;

	QMutexLocker locker(mutex);

	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	if (!pm) return -1;

	UAVObjectManager *om = pm->getObject<UAVObjectManager>();
	if (!om) return -2;

	UAVDataObject *obj = dynamic_cast<UAVDataObject *>(om->getObject(QString("HomeLocation")));
	if (!obj) return -3;

//	obj->requestUpdate();

	field = obj->getField("Set");
	if (!field) return -4;
	set = field->getValue().toBool();

	field = obj->getField("Latitude");
	if (!field) return -5;
	LLA[0] = field->getDouble() * 1e-7;

	field = obj->getField("Longitude");
	if (!field) return -6;
	LLA[1] = field->getDouble() * 1e-7;

	field = obj->getField("Altitude");
	if (!field) return -7;
	LLA[2] = field->getDouble();

	field = obj->getField(QString("ECEF"));
	if (!field) return -8;
	for (int i = 0; i < 3; i++)
		ECEF[i] = field->getDouble(i);

	field = obj->getField(QString("RNE"));
	if (!field) return -9;
	for (int i = 0; i < 9; i++)
		RNE[i] = field->getDouble(i);

	field = obj->getField(QString("Be"));
	if (!field) return -10;
	for (int i = 0; i < 3; i++)
		Be[i] = field->getDouble(i);

	return 0;	// OK
}

// ******************************
// GPS

int UAVObjectUtilManager::getGPSPosition(double LLA[3])
{
	UAVObjectField *field;

	QMutexLocker locker(mutex);

	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	if (!pm) return -1;

	UAVObjectManager *om = pm->getObject<UAVObjectManager>();
	if (!om) return -2;

	UAVDataObject *obj = dynamic_cast<UAVDataObject *>(om->getObject(QString("GPSPosition")));
	if (!obj) return -3;

//	obj->requestUpdate();

	field = obj->getField(QString("Latitude"));
	if (!field) return -4;
	LLA[0] = field->getDouble() * 1e-7;

	field = obj->getField(QString("Longitude"));
	if (!field) return -5;
	LLA[1] = field->getDouble() * 1e-7;

	field = obj->getField(QString("Altitude"));
	if (!field) return -6;
	LLA[2] = field->getDouble();

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
// telemetry port

int UAVObjectUtilManager::setTelemetrySerialPortSpeed(QString speed, bool save_to_sdcard)
{
	UAVObjectField *field;

	QMutexLocker locker(mutex);

	// ******************
	// save the new settings

	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	if (!pm) return -1;

	UAVObjectManager *om = pm->getObject<UAVObjectManager>();
	if (!om) return -2;

	UAVDataObject *obj = dynamic_cast<UAVDataObject *>(om->getObject(QString("/*TelemetrySettings*/")));
	if (!obj) return -3;

	field = obj->getField(QString("Speed"));
	if (!field) return -4;
	field->setValue(speed);

	obj->updated();

	// ******************
	// save the new setting to SD card

	if (save_to_sdcard)
		saveObjectToSD(obj);

	// ******************

	return 0;	// OK
}

int UAVObjectUtilManager::getTelemetrySerialPortSpeed(QString &speed)
{
	UAVObjectField *field;

	QMutexLocker locker(mutex);

	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	if (!pm) return -1;

	UAVObjectManager *om = pm->getObject<UAVObjectManager>();
	if (!om) return -2;

	UAVDataObject *obj = dynamic_cast<UAVDataObject *>(om->getObject(QString("TelemetrySettings")));
	if (!obj) return -3;

//	obj->requestUpdate();

	field = obj->getField(QString("Speed"));
	if (!field) return -4;
	speed = field->getValue().toString();

	return 0;	// OK
}

int UAVObjectUtilManager::getTelemetrySerialPortSpeeds(QComboBox *comboBox)
{
	UAVObjectField *field;

	QMutexLocker locker(mutex);

	if (!comboBox) return -1;

	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	if (!pm) return -2;

	UAVObjectManager *om = pm->getObject<UAVObjectManager>();
	if (!om) return -3;

	UAVDataObject *obj = dynamic_cast<UAVDataObject *>(om->getObject(QString("TelemetrySettings")));
	if (!obj) return -4;

//	obj->requestUpdate();

	field = obj->getField(QString("Speed"));
	if (!field) return -5;
	comboBox->addItems(field->getOptions());

	return 0;	// OK
}

// ******************************
