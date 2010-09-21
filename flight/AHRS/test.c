#include "inc/insgps.h"
#include "stdio.h"
#include "math.h"

extern struct NavStruct Nav;
extern float X[13];

int main()
{
	float gyro[3] = { 2.47, -0.25, 7.71 }, accel[3] = {
	-1.02, 0.70, -10.11}, dT = 0.04, mags[3] = {
	-50, -180, -376};
	float Pos[3] = { 0, 0, 0 }, Vel[3] = {
	0, 0, 0}, BaroAlt = 2.66, Speed = 4.4, Heading = 0;
	float yaw;
	int i, j;

	INSGPSInit();

	for (i = 0; i < 10000000; i++) {
		INSPrediction(gyro, accel, dT);
		//MagCorrection(mags);
		FullCorrection(mags, Pos, Vel, BaroAlt);
		yaw =
		    atan2((float)2 *
			  (Nav.q[0] * Nav.q[3] + Nav.q[1] * Nav.q[2]),
			  (float)(1 -
				  2 * (Nav.q[2] * Nav.q[2] +
				       Nav.q[3] * Nav.q[3]))) * 180 / M_PI;

		printf("%0.3f ", yaw);
		for (j = 0; j < 13; j++)
			printf("%f ", X[j]);
		printf("\r\n");
	}
	return 0;
}
