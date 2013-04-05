/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotLibraries OpenPilot System Libraries
 * @{
 * @file       alarms.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Include file of the alarm library
 * @see        The GNU Public License (GPL) Version 3
 *
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
#ifndef ALARMS_H
#define ALARMS_H

#include "systemalarms.h"
#define SYSTEMALARMS_ALARM_DEFAULT SYSTEMALARMS_ALARM_UNINITIALISED

int32_t AlarmsInitialize(void);
int32_t AlarmsSet(SystemAlarmsAlarmElem alarm, SystemAlarmsAlarmOptions severity);
SystemAlarmsAlarmOptions AlarmsGet(SystemAlarmsAlarmElem alarm);
int32_t AlarmsDefault(SystemAlarmsAlarmElem alarm);
void AlarmsDefaultAll();
int32_t AlarmsClear(SystemAlarmsAlarmElem alarm);
void AlarmsClearAll();
int32_t AlarmsHasWarnings();
int32_t AlarmsHasErrors();
int32_t AlarmsHasCritical();

#endif // ALARMS_H

/**
 * @}
 * @}
 */
