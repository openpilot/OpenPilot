/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup StabilizationModule Stabilization Module
 * @brief cruisecontrol mode
 * @note This file implements the logic for a cruisecontrol
 * @{
 *
 * @file       cruisecontrol.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Attitude stabilization module.
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

#include <openpilot.h>
#include <stabilization.h>
#include <attitudestate.h>
#include <sin_lookup.h>

static float cruisecontrol_factor = 1.0f;


static inline float CruiseControlLimitThrust(float thrust)
{
    // limit to user specified absolute max thrust
    return boundf(thrust, stabSettings.cruiseControl.min_thrust, stabSettings.cruiseControl.max_thrust);
}

// assumes 1.0 <= factor <= 100.0
// a factor of less than 1.0 could make it return a value less than stabSettings.cruiseControl.min_thrust
// CP helis need to have min_thrust=-1
//
// multicopters need to have min_thrust=0.05 or so
// values below that will not be subject to max / min limiting
// that means thrust can be less than min
// that means multicopter motors stop spinning at low stick
static inline float CruiseControlFactorToThrust(float factor, float thrust)
{
    // don't touch thrust if it's less than min_thrust
    // without that	 test, quadcopter props will spin up
    // to min thrust even at zero throttle stick
    // if Cruise Control is enabled on this flight switch position
    if (thrust > stabSettings.cruiseControl.min_thrust) {
        return CruiseControlLimitThrust(thrust * factor);
    }
    return thrust;
}

static float CruiseControlAngleToFactor(float angle)
{
    float factor;

    // avoid singularity
    if (angle > 89.999f && angle < 90.001f) {
        factor = stabSettings.settings.CruiseControlMaxPowerFactor;
    } else {
        // the simple bank angle boost calculation that Cruise Control revolves around
        factor = 1.0f / fabsf(cos_lookup_deg(angle));
        // factor in the power trim, no effect at 1.0, linear effect increases with factor
        factor = (factor - 1.0f) * stabSettings.cruiseControl.power_trim + 1.0f;
        // limit to user specified max power multiplier
        if (factor > stabSettings.settings.CruiseControlMaxPowerFactor) {
            factor = stabSettings.settings.CruiseControlMaxPowerFactor;
        }
    }
    return factor;
}


void cruisecontrol_compute_factor(AttitudeStateData *attitude, float thrustDemand)
{
    static float previous_angle;
    static uint32_t previous_time   = 0;
    static bool previous_time_valid = false;

    // For multiple, speedy flips this mainly strives to address the
    // fact that (due to thrust delay) thrust didn't average straight
    // down, but at an angle.  For less speedy flips it acts like it
    // used to.  It can be turned off by setting power delay to 0.

    // It takes significant time for the motors of a multi-copter to
    // spin up.  It takes significant time for the collective servo of
    // a CP heli to move from one end to the other.  Both of those are
    // modeled here as linear, i.e. twice as much change takes twice
    // as long. Given a correctly configured maximum delay time this
    // code calculates how far in advance to start the control
    // transition so that half way through the physical transition it
    // is just crossing the transition angle.
    // Example: Rotation rate = 360.  Full stroke delay = 0.2
    // Transition angle 90 degrees.  Start the transition 0.1 second
    // before 90 degrees (36 degrees at 360 deg/sec) and it will be
    // complete 0.1 seconds after 90 degrees.

    // Note that this code only handles the transition to/from inverted
    // thrust.  It doesn't handle the case where thrust is changed a
    // lot in a small angle range when that range is close to 90 degrees.
    // It doesn't handle the small constant "system delay" caused by the
    // delay between reading sensors and actuators beginning to respond.
    // It also assumes that the pilot is holding the throttle constant;
    // when the pilot does change the throttle, the compensation is
    // simply recalculated.

    // This implementation of future thrust isn't perfect.  That would
    // probably require an iterative procedure for solving a
    // transcendental equation of the form linear(x) = 1/cos(x).  It's
    // shortcomings generally don't hurt anything and work better than
    // without it.  It is designed to work perfectly if the pilot is
    // using full thrust during flips and it is only activated if 70% or
    // greater thrust is used.

    uint32_t time = PIOS_DELAY_GetuS();

    // Get roll and pitch angles, calculate combined angle, and begin
    // the general algorithm.
    // Example: 45 degrees roll plus 45 degrees pitch = 60 degrees
    // Do it every 8th iteration to save CPU.
    if (time != previous_time || previous_time_valid == false) {
        float angle, angle_unmodified;

        // spherical right triangle
        // 0.0 <= angle <= 180.0
        angle_unmodified = angle = RAD2DEG(acosf(cos_lookup_deg(attitude->Roll)
                                                 * cos_lookup_deg(attitude->Pitch)));

        // Calculate rate as a combined (roll and pitch) bank angle
        // change; in degrees per second.  Rate is calculated over the
        // most recent 8 loops through stabilization.  We could have
        // asked the gyros.  This is probably cheaper.
        if (previous_time_valid) {
            float rate;

            // rate can be negative.
            rate = (angle - previous_angle) / ((float)(time - previous_time) / 1000000.0f);

            // Define "within range" to be those transitions that should
            // be executing now.  Recall that each impulse transition is
            // spread out over a range of time / angle.

            // There is only one transition and the high power level for
            // it is either:
            // 1/fabsf(cos(angle)) * current thrust
            // or max power factor * current thrust
            // or full thrust
            // You can cross the transition with angle either increasing
            // or decreasing (rate positive or negative).

            // Thrust is never boosted for negative values of
            // thrustDemand (negative stick values)
            //
            // When the aircraft is upright, thrust is always boosted
            // . for positive values of thrustDemand
            // When the aircraft is inverted, thrust is sometimes
            // . boosted or reversed (or combinations thereof) or zeroed
            // . for positive values of thrustDemand
            // It depends on the inverted power settings.
            // Of course, you can set MaxPowerFactor to 1.0 to
            // . effectively disable boost.
            if (thrustDemand > 0.0f) {
                // to enable the future thrust calculations, make sure
                // there is a large enough transition that the result
                // will be roughly on vs. off; without that, it can
                // exaggerate the length of time the inverted to upright
                // transition holds full throttle and reduce the length
                // of time for full throttle when going upright to inverted.
                if (thrustDemand > 0.7f) {
                    float thrust;

                    thrust = CruiseControlFactorToThrust(CruiseControlAngleToFactor((float)stabSettings.settings.CruiseControlMaxAngle), thrustDemand);

                    // determine if we are in range of the transition

                    // given the thrust at max_angle and thrustDemand
                    // (typically close to 1.0), change variable 'thrust' to
                    // be the proportion of the largest thrust change possible
                    // that occurs when going into inverted mode.
                    // Example: 'thrust' is 0.8  A quad has min_thrust set
                    // to 0.05  The difference is 0.75.  The largest possible
                    // difference with this setup is 0.9 - 0.05 = 0.85, so
                    // the proportion is 0.75/0.85
                    // That is nearly a full throttle stroke.
                    // the 'thrust' variable is non-negative here
                    switch (stabSettings.settings.CruiseControlInvertedPowerOutput) {
                    case STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDPOWEROUTPUT_ZERO:
                        // normal multi-copter case, stroke is max to zero
                        // technically max to constant min_thrust
                        // can be used by CP
                        thrust = (thrust - CruiseControlLimitThrust(0.0f)) / stabSettings.cruiseControl.thrust_difference;
                        break;
                    case STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDPOWEROUTPUT_NORMAL:
                        // reversed but not boosted
                        // : CP heli case, stroke is max to -stick
                        // : thrust = (thrust - CruiseControlLimitThrust(-thrustDemand)) / stabSettings.cruiseControl.thrust_difference;
                        // else it is both unreversed and unboosted
                        // : simply turn off boost, stroke is max to +stick
                        // : thrust = (thrust - CruiseControlLimitThrust(thrustDemand)) / stabSettings.cruiseControl.thrust_difference;
                        thrust = (thrust - CruiseControlLimitThrust(
                                      (stabSettings.settings.CruiseControlInvertedThrustReversing
                                       == STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDTHRUSTREVERSING_REVERSED)
                                      ? -thrustDemand
                                      : thrustDemand)) / stabSettings.cruiseControl.thrust_difference;
                        break;
                    case STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDPOWEROUTPUT_BOOSTED:
                        // if boosted and reversed
                        if (stabSettings.settings.CruiseControlInvertedThrustReversing
                            == STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDTHRUSTREVERSING_REVERSED) {
                            // CP heli case, stroke is max to min
                            thrust = (thrust - CruiseControlFactorToThrust(-CruiseControlAngleToFactor((float)stabSettings.settings.CruiseControlMaxAngle), thrustDemand)) / stabSettings.cruiseControl.thrust_difference;
                        }
                        // else it is boosted and unreversed so the throttle doesn't change
                        else {
                            // CP heli case, no transition, so stroke is zero
                            thrust = 0.0f;
                        }
                        break;
                    }

                    // 'thrust' is now the proportion of max stroke
                    // multiply this proportion of max stroke,
                    // times the max stroke time, to get this stroke time
                    // we only want half of this time before the transition
                    // (and half after the transition)
                    thrust *= stabSettings.cruiseControl.half_power_delay;
                    // 'thrust' is now the length of time for this stroke
                    // multiply that times angular rate to get the lead angle
                    thrust *= fabsf(rate);
                    // if the transition is within range we use it,
                    // else we just use the current calculated thrust
                    if ((float)stabSettings.settings.CruiseControlMaxAngle - thrust <= angle
                        && angle <= (float)stabSettings.settings.CruiseControlMaxAngle + thrust) {
                        // default to a little above max angle
                        angle = (float)stabSettings.settings.CruiseControlMaxAngle + 0.01f;
                        // if roll direction is downward
                        // then thrust value is taken from below max angle
                        // by the code that knows about the transition angle
                        if (rate < 0.0f) {
                            angle -= 0.02f;
                        }
                    }
                } // if thrust > 0.7; else just use the angle we already calculated
                cruisecontrol_factor = CruiseControlAngleToFactor(angle);
            } else { // if thrust > 0 set factor from angle; else
                cruisecontrol_factor = 1.0f;
            }

            if (angle >= (float)stabSettings.settings.CruiseControlMaxAngle) {
                switch (stabSettings.settings.CruiseControlInvertedPowerOutput) {
                case STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDPOWEROUTPUT_ZERO:
                    cruisecontrol_factor = 0.0f;
                    break;
                case STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDPOWEROUTPUT_NORMAL:
                    cruisecontrol_factor = 1.0f;
                    break;
                case STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDPOWEROUTPUT_BOOSTED:
                    // no change, leave factor >= 1.0 alone
                    break;
                }
                if (stabSettings.settings.CruiseControlInvertedThrustReversing
                    == STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDTHRUSTREVERSING_REVERSED) {
                    cruisecontrol_factor = -cruisecontrol_factor;
                }
            }
        } // if previous_time_valid i.e. we've got a rate; else leave (angle and) factor alone
        previous_time  = time;
        previous_time_valid = true;
        previous_angle = angle_unmodified;
    } // every 8th time
}

float cruisecontrol_apply_factor(float raw)
{
    if (stabSettings.settings.CruiseControlMaxPowerFactor > 0.0001f) {
        raw = CruiseControlFactorToThrust(cruisecontrol_factor, raw);
    }
    return raw;
}
