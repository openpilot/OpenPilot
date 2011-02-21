/**
 ******************************************************************************
 *
 * @file       uavobjectmanager.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 * @brief      The UAVUObjects GCS plugin
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
#ifndef UAVOBJECTUTILMANAGER_H
#define UAVOBJECTUTILMANAGER_H

#include "uavobjectutil_global.h"

#include <QtGlobal>
#include <QObject>
#include <QMutex>

class UAVOBJECTUTIL_EXPORT UAVObjectUtilManager: public QObject
{
    Q_OBJECT

public:
	UAVObjectUtilManager();
	~UAVObjectUtilManager();

	int setHomeLocation(double LLA[3], bool save_to_sdcard);
	int getHomeLocation(bool &set, double LLA[3]);
	int getHomeLocation(bool &set, double LLA[3], double ECEF[3], double RNE[9], double Be[3]);

private:
	QMutex *mutex;

};


#endif
