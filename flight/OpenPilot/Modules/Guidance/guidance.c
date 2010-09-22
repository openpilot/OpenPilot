/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup GuidanceModule Guidance Module
 * @brief Guidance PID loops in an airframe type independent manner
 * @note This object updates the @ref AttitudeDesired "Attitude Desired" based on the 
 * PID loops on the craft position, speed and course 
 * @{ 
 *
 * @file       guidance.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Attitude guidance module.
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

#include "openpilot.h"
#include "guidance.h"
#include "guidancesettings.h"
#include "attitudedesired.h"
#include "positiondesired.h"
#include "positionactual.h"
#include "attitudeactual.h"
#include "manualcontrolcommand.h"
#include "systemsettings.h"


// Private constants
#define STACK_SIZE configMINIMAL_STACK_SIZE
#define TASK_PRIORITY (tskIDLE_PRIORITY+4)
#define LATERAL_INTEGRAL_LIMIT 0.5
#define COURSE_INTEGRAL_LIMIT 0.5
#define ENERGY_INTEGRAL_LIMIT 0.5
#define SPEED_INTEGRAL_LIMIT 0.5
#define DEG2RAD ( M_PI / 180.0 )
#define GEE 9.81

// Private types

// Private variables
static xTaskHandle taskHandle;

// Private functions
static void guidanceTask(void* parameters);
static float bound(float val, float min, float max);
static float angleDifference(float val);

/**
 * Module initialization
 */
int32_t GuidanceInitialize()
{
    // Initialize variables

    // Start main task
    xTaskCreate(guidanceTask, (signed char*)"Guidance", STACK_SIZE, NULL, TASK_PRIORITY, &taskHandle);

    return 0;
}

/**
 * Module task
 */
static void guidanceTask(void* parameters)
{
    GuidanceSettingsData guidanceSettings;
    AttitudeDesiredData attitudeDesired;
    AttitudeActualData attitudeActual;
    PositionActualData positionActual;
    PositionDesiredData positionDesired;

    ManualControlCommandData manualControl;
    SystemSettingsData systemSettings;
    portTickType lastSysTime;
    // declarations...
    float lateralError, lateralErrorLast, lateralDerivative, lateralIntegralLimit, lateralIntegral;
    float courseError, courseErrorLast, courseDerivative, courseIntegralLimit, courseIntegral;
    float speedError, speedErrorLast, speedDerivative, speedIntegralLimit, speedIntegral;
    float energyError, energyErrorLast, energyDerivative, energyIntegralLimit, energyIntegral;
    float sinAlpha, cosAlpha;

    // Initialize
    lateralIntegral  = 0.0;
    courseIntegral   = 0.0;
    speedIntegral    = 0.0;
    energyIntegral   = 0.0;

    lateralErrorLast = 0.0;
    courseErrorLast  = 0.0;
    speedErrorLast   = 0.0;
    energyErrorLast  = 0.0;

    // Main task loop
    lastSysTime = xTaskGetTickCount();
    while (1)
    {
        // Read settings and other objects
        GuidanceSettingsGet(&guidanceSettings);
        SystemSettingsGet(&systemSettings);
        ManualControlCommandGet(&manualControl);
        PositionDesiredGet(&positionDesired);
        PositionActualGet(&positionActual);
        AttitudeActualGet(&attitudeActual);

        // lateral PID Loop
        // error is the distance between the current position and the imaginary line going through PositionDesired.NED
        // at PositionDesired.Heading degrees course

        sinAlpha = sin(positionDesired.Heading);
        cosAlpha = cos(positionDesired.Heading);
        lateralError         = ( 
                                   - sinAlpha * ( positionDesired.NED[0] - positionActual.NED[0] )
                                   + cosAlpha * ( positionDesired.NED[1] - positionActual.NED[1]  )
                                ) / ( sinAlpha*sinAlpha + cosAlpha*cosAlpha );
        lateralDerivative    = lateralError - lateralErrorLast;
        lateralErrorLast     = lateralError;
        lateralIntegralLimit = LATERAL_INTEGRAL_LIMIT / guidanceSettings.LateralKi;
        lateralIntegral      = bound(lateralIntegral+lateralError*guidanceSettings.UpdatePeriod,
                                -lateralIntegralLimit,lateralIntegralLimit);
        attitudeDesired.Yaw  = angleDifference( bound( ( guidanceSettings.LateralKp*lateralError +
                                                         guidanceSettings.LateralKi*lateralIntegral +
                                                         guidanceSettings.LateralKd*lateralDerivative
                                                       ),-90,90
                                                     ) + positionDesired.Heading
                                              );

        if (attitudeDesired.Yaw<0) attitudeDesired.Yaw+=360;



        // course PID Loop
        // very straighforward
        courseError          = angleDifference( attitudeDesired.Yaw - positionActual.Heading );
        courseDerivative     = courseError - courseErrorLast;
        courseErrorLast      = courseError;
        courseIntegralLimit  = COURSE_INTEGRAL_LIMIT / guidanceSettings.CourseKi;
        courseIntegral       = bound(courseIntegral+courseError*guidanceSettings.UpdatePeriod,
                                -courseIntegralLimit,courseIntegralLimit);
        attitudeDesired.Roll = bound( ( guidanceSettings.CourseKp*courseError +
                                        guidanceSettings.CourseKi*courseIntegral +
                                        guidanceSettings.CourseKd*courseDerivative
                                      ),-guidanceSettings.RollMax,guidanceSettings.RollMax
                                    );


        // speed PID loop
        // since desired value is given as groundspeed, but our limits affect airspeed, translation is necessary
        // we assume a constant (wind) offset between the two
        // (this is not completely correct since there might be an air pressure dependent linear relationship, too
        //  but this puts only a linear multiplication factor on our error)
        speedError            = bound(
                                        positionDesired.Groundspeed + 
                                        (positionActual.Airspeed-positionActual.Groundspeed)
                                       ,guidanceSettings.SpeedMin,guidanceSettings.SpeedMax
                                     ) - positionActual.Airspeed;
        speedDerivative       = speedError - speedErrorLast;
        speedErrorLast        = speedError;
        speedIntegralLimit    = SPEED_INTEGRAL_LIMIT / guidanceSettings.SpeedKi;
        speedIntegral         = bound(speedIntegral+speedError*guidanceSettings.UpdatePeriod,
                                 -speedIntegralLimit,speedIntegralLimit);
        attitudeDesired.Pitch = bound( -( guidanceSettings.SpeedKp*speedError +
                                          guidanceSettings.SpeedKi*speedIntegral +
                                          guidanceSettings.SpeedKd*speedDerivative
                                        ),guidanceSettings.PitchMin,guidanceSettings.PitchMax
                                     );

        // energy PID loop - flight energy = mass_factor*(speed^2+g*altitude) - using mass_factor=1
        // the desired energy is calculated from the desired airspeed (which has been calculated above)
        // and the desired altitude
        energyError         =  (
                                ( speedError + positionActual.Airspeed ) * ( speedError + positionActual.Airspeed )
                                + GEE * -positionDesired.NED[3]
                              ) - (
                                positionActual.Airspeed * positionActual.Airspeed + positionActual.Climbrate * positionActual.Climbrate
                                + GEE * -positionActual.NED[3]
                               );
        energyDerivative    = energyError - energyErrorLast;
        energyErrorLast     = energyError;
        energyIntegralLimit = SPEED_INTEGRAL_LIMIT / guidanceSettings.EnergyKi;
        energyIntegral      = bound(energyIntegral+energyError*guidanceSettings.UpdatePeriod,
                               -energyIntegralLimit,energyIntegralLimit);
        attitudeDesired.Throttle = bound( ( guidanceSettings.EnergyKp*energyError +
                                            guidanceSettings.EnergyKi*energyIntegral +
                                            guidanceSettings.EnergyKd*energyDerivative
                                           ),guidanceSettings.ThrottleMin,guidanceSettings.ThrottleMax
                                        );


        // adapt roll in case of negative pitch demand
        if (attitudeDesired.Pitch < attitudeActual.Pitch) {
            if (attitudeDesired.Pitch <= attitudeActual.Pitch - guidanceSettings.PitchRollEpsilon) {
                // in case of heavy push, reverse roll
                attitudeDesired.Roll = -attitudeDesired.Roll;
            } else {
                // otherwise linear interpolation between Roll and -Roll
                attitudeDesired.Roll -= 2.0 * attitudeDesired.Roll * 
                              ( attitudeActual.Pitch - attitudeDesired.Pitch )
                              / guidanceSettings.PitchRollEpsilon;
            }
        }

        // Write actuator desired (if not in manual mode)
        if ( manualControl.FlightMode == MANUALCONTROLCOMMAND_FLIGHTMODE_AUTO )
        {
            AttitudeDesiredSet(&attitudeDesired);
        }
        else
        {
            lateralIntegral  = 0.0;
            courseIntegral   = 0.0;
            speedIntegral    = 0.0;
            energyIntegral   = 0.0;

            lateralErrorLast = 0.0;
            courseErrorLast  = 0.0;
            speedErrorLast   = 0.0;
            energyErrorLast  = 0.0;
        }

        // Clear alarms
        AlarmsClear(SYSTEMALARMS_ALARM_GUIDANCE);

        // Wait until next update
        vTaskDelayUntil(&lastSysTime, guidanceSettings.UpdatePeriod / portTICK_RATE_MS );
    }
}

/**
 * Bound input value between limits
 */
static float bound(float val, float min, float max)
{
    if ( val < min )
    {
        val = min;
    }
    else if ( val > max )
    {
        val = max;
    }
    return val;
}

/**
 * Fix result of angular differences
 */
static float angleDifference(float val)
{
    while ( val < -180.0 )
    {
        val += 360.0;
    }
    while ( val > 180.0 )
    {
        val -= 360.0;
    }
    return val;
}

/** 
  * @}
  * @}
  */
