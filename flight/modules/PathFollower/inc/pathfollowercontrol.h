/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup PathFollower CONTROL interface class
 * @brief CONTROL interface class for pathfollower goal implementations
 * @{
 *
 * @file       pathfollowercontrol.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Interface class for controllers
 *
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
#ifndef PATHFOLLOWERCONTROL_H
#define PATHFOLLOWERCONTROL_H
class PathFollowerControl {
public:
    virtual void Activate(void)   = 0;
    virtual void Deactivate(void) = 0;
    virtual void SettingsUpdated(void)  = 0;
    virtual void UpdateAutoPilot(void)  = 0;
    virtual void ObjectiveUpdated(void) = 0;
    virtual uint8_t Mode(void) = 0;
    static int32_t Initialize(PathDesiredData *ptr_pathDesired,
                              FlightStatusData *ptr_flightStatus,
                              PathStatusData *ptr_pathStatus);
protected:
    static PathDesiredData *pathDesired;
    static FlightStatusData *flightStatus;
    static PathStatusData *pathStatus;
};

#endif // PATHFOLLOWERCONTROL_H
