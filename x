diff --git a/flight/modules/AltitudeHold/altitudehold.c b/flight/modules/AltitudeHold/altitudehold.c
index a886de6..cb1f624 100644
--- a/flight/modules/AltitudeHold/altitudehold.c
+++ b/flight/modules/AltitudeHold/altitudehold.c
@@ -108,8 +108,8 @@ int32_t AltitudeHoldInitialize()
 MODULE_INITCALL(AltitudeHoldInitialize, AltitudeHoldStart);
 
 float tau;
-float throttleIntegral;
-float velocity;
+float altitudeIntegral;
+float velocityIntegral;
 float decay;
 float velocity_decay;
 bool running = false;
@@ -125,6 +125,8 @@ static void altitudeHoldTask(__attribute__((unused)) void *parameters)
 {
     AltitudeHoldDesiredData altitudeHoldDesired;
     StabilizationDesiredData stabilizationDesired;
+    AltHoldSmoothedData altHold;
+    float q[4], Rbe[3][3], fblimit = 0;
 
     portTickType thisTime, lastUpdateTime;
     UAVObjEvent ev;
@@ -155,7 +157,7 @@ static void altitudeHoldTask(__attribute__((unused)) void *parameters)
         // Wait until the AttitudeRaw object is updated, if a timeout then go to failsafe
         if (xQueueReceive(queue, &ev, 100 / portTICK_RATE_MS) != pdTRUE) {
             if (!running) {
-                throttleIntegral = 0;
+                altitudeIntegral = 0;
             }
 
             // Todo: Add alarm if it should be running
@@ -169,15 +171,20 @@ static void altitudeHoldTask(__attribute__((unused)) void *parameters)
             altitudeHoldFlightMode = flightMode == FLIGHTSTATUS_FLIGHTMODE_ALTITUDEHOLD || flightMode == FLIGHTSTATUS_FLIGHTMODE_ALTITUDEVARIO;
             if (altitudeHoldFlightMode && !running) {
                 // Copy the current throttle as a starting point for integral
-                StabilizationDesiredThrottleGet(&throttleIntegral);
-                switchThrottle = throttleIntegral;
+            	float initThrottle;
+                StabilizationDesiredThrottleGet(&initThrottle);
+                initThrottle *= Rbe[2][2]; // rotate into earth frame
+                if (initThrottle > 1) {
+                	initThrottle = 1;
+                } else if (initThrottle < 0) {
+                	initThrottle = 0;
+                }
                 error    = 0;
-                velocity = 0;
+                altitudeHoldDesired.Velocity = 0;
+                altitudeHoldDesired.Altitude = altHold.Altitude;
+                altitudeIntegral = altHold.Altitude * altitudeHoldSettings.Kp + initThrottle;
+                velocityIntegral = 0;
                 running  = true;
-
-                AltHoldSmoothedData altHold;
-                AltHoldSmoothedGet(&altHold);
-                starting_altitude = altHold.Altitude;
             } else if (!altitudeHoldFlightMode) {
                 running = false;
                 lastAltitudeHoldDesiredUpdate = PIOS_DELAY_GetRaw();
@@ -242,17 +249,12 @@ static void altitudeHoldTask(__attribute__((unused)) void *parameters)
 
             x[0] = baro.Altitude;
             // rotate avg accels into earth frame and store it
-            if (1) {
-                float q[4], Rbe[3][3];
-                q[0] = attitudeState.q1;
-                q[1] = attitudeState.q2;
-                q[2] = attitudeState.q3;
-                q[3] = attitudeState.q4;
-                Quaternion2R(q, Rbe);
-                x[1] = -(Rbe[0][2] * accelState.x + Rbe[1][2] * accelState.y + Rbe[2][2] * accelState.z + 9.81f);
-            } else {
-                x[1] = -accelState.z + 9.81f;
-            }
+            q[0] = attitudeState.q1;
+            q[1] = attitudeState.q2;
+            q[2] = attitudeState.q3;
+            q[3] = attitudeState.q4;
+            Quaternion2R(q, Rbe);
+            x[1] = -(Rbe[0][2] * accelState.x + Rbe[1][2] * accelState.y + Rbe[2][2] * accelState.z + 9.81f);
 
             dT = PIOS_DELAY_DiffuS(timeval) / 1.0e6f;
             timeval = PIOS_DELAY_GetRaw();
@@ -342,7 +344,6 @@ static void altitudeHoldTask(__attribute__((unused)) void *parameters)
                 V[3][3] = -K[3][0] * P[2][3] - P[3][3] * (K[3][0] - 1.0f);
             }
 
-            AltHoldSmoothedData altHold;
             AltHoldSmoothedGet(&altHold);
             altHold.Altitude = z[0];
             altHold.Velocity = z[1];
@@ -363,11 +364,11 @@ static void altitudeHoldTask(__attribute__((unused)) void *parameters)
                 continue;
             }
 
-            // Compute the altitude error
-            error = (starting_altitude + altitudeHoldDesired.Altitude) - altHold.Altitude;
-
-            // Compute integral off altitude error
-            throttleIntegral += error * altitudeHoldSettings.Ki * dT;
+            altitudeHoldDesired.Altitude += altitudeHoldDesired.Velocity * dT;
+            AltitudeHoldDesiredAltitudeSet(&altitudeHoldDesired.Altitude);
+            // Compute altitude and velocity integral
+            altitudeIntegral += (altitudeHoldDesired.Altitude - altHold.Altitude - fblimit) * altitudeHoldSettings.AltitudeKi * dT;
+            velocityIntegral += (altitudeHoldDesired.Velocity - altHold.Velocity - fblimit) * altitudeHoldSettings.VelocityKi * dT;
 
             // Only update stabilizationDesired less frequently
             if ((thisTime - lastUpdateTime) < 20) {
@@ -379,13 +380,23 @@ static void altitudeHoldTask(__attribute__((unused)) void *parameters)
             // Instead of explicit limit on integral you output limit feedback
             StabilizationDesiredGet(&stabilizationDesired);
             if (!enterFailSafe) {
-                stabilizationDesired.Throttle = error * altitudeHoldSettings.Kp + throttleIntegral -
-                                                altHold.Velocity * altitudeHoldSettings.Kd - altHold.Accel * altitudeHoldSettings.Ka;
+                stabilizationDesired.Throttle = altitudeIntegral + velocityIntegral
+                		                        - altHold.Altitude * altitudeHoldSettings.Kp
+                                                - altHold.Velocity * altitudeHoldSettings.Kd
+                                                - altHold.Accel * altitudeHoldSettings.Ka;
+                // scale up throttle to compensate for roll/pitch angle but limit this to 60 deg (cos(60) == 0.5) to prevent excessive scaling
+                float throttlescale = Rbe[2][2] < 0.5f ? 0.5f : Rbe[2][2];
+                stabilizationDesired.Throttle /= throttlescale;
+                fblimit = 0;
                 if (stabilizationDesired.Throttle > 1) {
-                    throttleIntegral -= (stabilizationDesired.Throttle - 1);
+                 //   altitudeIntegral -= (stabilizationDesired.Throttle - 1);
+                 //   velocityIntegral -= (stabilizationDesired.Throttle - 1);
+                	fblimit = stabilizationDesired.Throttle - 1;
                     stabilizationDesired.Throttle = 1;
                 } else if (stabilizationDesired.Throttle < 0) {
-                    throttleIntegral -= stabilizationDesired.Throttle;
+                 //   altitudeIntegral -= stabilizationDesired.Throttle;
+                 //   velocityIntegral -= stabilizationDesired.Throttle;
+                	fblimit = stabilizationDesired.Throttle;
                     stabilizationDesired.Throttle = 0;
                 }
             } else {
