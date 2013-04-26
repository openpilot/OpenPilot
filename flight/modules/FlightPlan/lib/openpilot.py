#
# *****************************************************************************
# *
# * @file       openpilot.py
# * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
# * @brief      Python OpenPilot library, gives FlightPlan scripts access to 
# *             RTOS and other functions 
# *
# * @see        The GNU Public License (GPL) Version 3
# *
# *****************************************************************************
# *
# * This program is free software; you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation; either version 3 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful, but
# * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# * for more details.
# *
# * You should have received a copy of the GNU General Public License along
# * with this program; if not, write to the Free Software Foundation, Inc.,
# * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

"""__NATIVE__
#include "openpilot.h"
#include "flightplanstatus.h"
#include "flightplancontrol.h"
"""

# Delay (suspend VM thread) for timeToDelayMs ms
def delay(timeToDelayMs):
	"""__NATIVE__
	pPmObj_t pobj;
	PmReturn_t retval;
	portTickType timeToDelayTicks;

	// Check number of arguments
  if (NATIVE_GET_NUM_ARGS() != 1)
  {
		PM_RAISE(retval, PM_RET_EX_TYPE);
		return retval;
 	}

	// Get argument
	pobj = NATIVE_GET_LOCAL(0);
	if ( OBJ_GET_TYPE(pobj) == OBJ_TYPE_INT )
		timeToDelayTicks = (portTickType)(((pPmInt_t) pobj)->val) / portTICK_RATE_MS;
	else if ( OBJ_GET_TYPE(pobj) == OBJ_TYPE_FLT )
		timeToDelayTicks = (portTickType)(((pPmFloat_t) pobj)->val) / portTICK_RATE_MS;
	else
	{
		PM_RAISE(retval, PM_RET_EX_TYPE);
		return retval;
	}
	 
	// Delay
	vTaskDelay(timeToDelayTicks);
 
	return PM_RET_OK;
	"""
	pass

# Same as delay() but will result in more exact periodic execution
# lastWakeTimeMs should be initialized with the system time.
# Example:
# timenow = openpilot.time()
# while 1:
#	  timenow = openpilot.delayUntil(timenow, 1000)
#
def delayUntil(lastWakeTimeMs, timeToDelayMs):
	"""__NATIVE__
	pPmObj_t pobj;
	pPmObj_t pobjret;
	PmReturn_t retval;
	portTickType lastWakeTimeTicks;
	portTickType timeToDelayTicks;

	// Check number of arguments
  if (NATIVE_GET_NUM_ARGS() != 2)
  {
		PM_RAISE(retval, PM_RET_EX_TYPE);
		return retval;
 	}

	// Get lastWakeTimeMs argument
	pobj = NATIVE_GET_LOCAL(0);
	if ( OBJ_GET_TYPE(pobj) == OBJ_TYPE_INT )
		lastWakeTimeTicks = (portTickType)(((pPmInt_t) pobj)->val)  / portTICK_RATE_MS;
	else if ( OBJ_GET_TYPE(pobj) == OBJ_TYPE_FLT )
		lastWakeTimeTicks = (portTickType)(((pPmFloat_t) pobj)->val)  / portTICK_RATE_MS;
	else
	{
		PM_RAISE(retval, PM_RET_EX_TYPE);
		return retval;
	}

	// Get timeToDelayMs argument
	pobj = NATIVE_GET_LOCAL(1);
	if ( OBJ_GET_TYPE(pobj) == OBJ_TYPE_INT )
		timeToDelayTicks = (portTickType)(((pPmInt_t) pobj)->val)  / portTICK_RATE_MS;
	else if ( OBJ_GET_TYPE(pobj) == OBJ_TYPE_FLT )
		timeToDelayTicks = (portTickType)(((pPmFloat_t) pobj)->val)  / portTICK_RATE_MS;
	else
	{
		PM_RAISE(retval, PM_RET_EX_TYPE);
		return retval;
	}
	 
	// Delay
	vTaskDelayUntil(&lastWakeTimeTicks, timeToDelayTicks);

  // Return an int object with the time value */
	retval = int_new((int32_t)(lastWakeTimeTicks*portTICK_RATE_MS), &pobjret);
  NATIVE_SET_TOS(pobjret);
  return retval;
	"""
	pass

# Update FlightPlanStatus debug fields
def debug(val1, val2):
	"""__NATIVE__
	pPmObj_t pobj;
	FlightPlanStatusData status;
	PmReturn_t retval;

	// Check number of arguments
	if (NATIVE_GET_NUM_ARGS() != 2)
 	{
		PM_RAISE(retval, PM_RET_EX_TYPE);
		return retval;
 	}

	// Get status object
	FlightPlanStatusGet(&status);	

	// Update debug1
	pobj = NATIVE_GET_LOCAL(0);
	if ( OBJ_GET_TYPE(pobj) == OBJ_TYPE_INT )
		status.Debug[0] = (float)(((pPmInt_t) pobj)->val);
	else if ( OBJ_GET_TYPE(pobj) == OBJ_TYPE_FLT )
		status.Debug[0] = (float)(((pPmFloat_t) pobj)->val);
	else
	{
		PM_RAISE(retval, PM_RET_EX_TYPE);
		return retval;
	}

	// Update debug2
	pobj = NATIVE_GET_LOCAL(1);
	if ( OBJ_GET_TYPE(pobj) == OBJ_TYPE_INT )
		status.Debug[1] = (float)(((pPmInt_t) pobj)->val);
	else if ( OBJ_GET_TYPE(pobj) == OBJ_TYPE_FLT )
		status.Debug[1] = (float)(((pPmFloat_t) pobj)->val);
	else
	{
		PM_RAISE(retval, PM_RET_EX_TYPE);
		return retval;
	}
	
	// Update status object 
	FlightPlanStatusSet(&status);
 
	return PM_RET_OK;
	"""
	pass

# Returns 1 if a stop request is pending. The script should periodically check 
# for stop requests and exit using sys.exit() if one is detected.
def hasStopRequest():
	"""__NATIVE__
	pPmObj_t pobjret;
	PmReturn_t retval;
	FlightPlanControlData control;
	uint32_t stopRequest;

	// Get control object
	FlightPlanControlGet(&control);

	// Check if a stop request is pending
	if (control.Command == FLIGHTPLANCONTROL_COMMAND_STOP)
		stopRequest = 1;
	else
		stopRequest = 0;

	// Return 
	retval = int_new((int32_t)(stopRequest), &pobjret);
  NATIVE_SET_TOS(pobjret);
  return retval;
	"""
	pass

# TODO: Wait for object updates in the event queue 
def waitForObjectUpdates(timeoutMs):
	pass




