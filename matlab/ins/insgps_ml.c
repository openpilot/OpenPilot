#include "math.h"
#include "mex.h"   
#include "insgps.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"

bool mlStringCompare(const mxArray * mlVal, char * cStr);
bool mlGetFloatArray(const mxArray * mlVal, float * dest, int numel);

// constants/macros/typdefs
#define NUMX 13			// number of states, X is the state vector
#define NUMW 9			// number of plant noise inputs, w is disturbance noise vector
#define NUMV 10			// number of measurements, v is the measurement noise vector
#define NUMU 6			// number of deterministic inputs, U is the input vector

extern float F[NUMX][NUMX], G[NUMX][NUMW], H[NUMV][NUMX];	// linearized system matrices
extern float Be[3];			// local magnetic unit vector in NED frame
extern float P[NUMX][NUMX], X[NUMX];	// covariance matrix and state vector
extern float Q[NUMW], R[NUMV];		// input noise and measurement noise variances
extern float K[NUMX][NUMV];		// feedback gain matrix
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	char * function_name;
	float accel_data[3];
	float gyro_data[3];
	float mag_data[3];
	float pos_data[3];
	float vel_data[3];
	float baro_data;
	float dT;

	//All code and internal function calls go in here!
	if(!mxIsChar(prhs[0])) {
		mexErrMsgTxt("First parameter must be name of a function\n");
		return;
	} 

	if(mlStringCompare(prhs[0], "INSGPSInit")) {
		INSGPSInit();
	} else 	if(mlStringCompare(prhs[0], "INSStatePrediction")) {

		if(nrhs != 4) {
			mexErrMsgTxt("Incorrect number of inputs for state prediction\n");
			return;
		}

		if(!mlGetFloatArray(prhs[1], gyro_data, 3) || 
			!mlGetFloatArray(prhs[2], accel_data, 3) ||
			!mlGetFloatArray(prhs[3], &dT, 1)) 
			return;

		INSStatePrediction(gyro_data, accel_data, dT);
		INSCovariancePrediction(dT);
	} else 	if(mlStringCompare(prhs[0], "INSFullCorrection")) {

		if(nrhs != 5) {
			mexErrMsgTxt("Incorrect number of inputs for correction\n");
			return;
		}

		if(!mlGetFloatArray(prhs[1], mag_data, 3) ||
				!mlGetFloatArray(prhs[2], pos_data, 3) ||
				!mlGetFloatArray(prhs[3], vel_data ,3) ||
				!mlGetFloatArray(prhs[4], &baro_data, 1)) {
			mexErrMsgTxt("Error with the input parameters\n");
			return;
		}

		FullCorrection(mag_data, pos_data, vel_data, baro_data);
	} else 	if(mlStringCompare(prhs[0], "INSMagCorrection")) {
		if(nrhs != 2) {
			mexErrMsgTxt("Incorrect number of inputs for correction\n");
			return;
		}

		if(!mlGetFloatArray(prhs[1], mag_data, 3)) {
			mexErrMsgTxt("Error with the input parameters\n");
			return;
		}

		MagCorrection(mag_data);
    } else 	if(mlStringCompare(prhs[0], "INSBaroCorrection")) {
		if(nrhs != 2) {
			mexErrMsgTxt("Incorrect number of inputs for correction\n");
			return;
		}

		if(!mlGetFloatArray(prhs[1], &baro_data, 1)) {
			mexErrMsgTxt("Error with the input parameters\n");
			return;
		}

		BaroCorrection(baro_data);
	} else 	if(mlStringCompare(prhs[0], "INSMagVelBaroCorrection")) {

		if(nrhs != 4) {
			mexErrMsgTxt("Incorrect number of inputs for correction\n");
			return;
		}

		if(!mlGetFloatArray(prhs[1], mag_data, 3) ||
				!mlGetFloatArray(prhs[2], vel_data ,3) ||
				!mlGetFloatArray(prhs[3], &baro_data, 1)) {
			mexErrMsgTxt("Error with the input parameters\n");
			return;
		}

		MagVelBaroCorrection(mag_data, vel_data, baro_data);
	} else 	if(mlStringCompare(prhs[0], "INSGpsCorrection")) {

		if(nrhs != 3) {
			mexErrMsgTxt("Incorrect number of inputs for correction\n");
			return;
		}

		if(!mlGetFloatArray(prhs[1], pos_data, 3) ||
				!mlGetFloatArray(prhs[2], vel_data ,3)) {
			mexErrMsgTxt("Error with the input parameters\n");
			return;
		}

		GpsCorrection(pos_data, vel_data);
	} else 	if(mlStringCompare(prhs[0], "INSVelBaroCorrection")) {

		if(nrhs != 3) {
			mexErrMsgTxt("Incorrect number of inputs for correction\n");
			return;
		}

		if(!mlGetFloatArray(prhs[1], vel_data, 3) ||
				!mlGetFloatArray(prhs[2], &baro_data, 1)) {
			mexErrMsgTxt("Error with the input parameters\n");
			return;
		}

		VelBaroCorrection(vel_data, baro_data);
	} else if (mlStringCompare(prhs[0], "INSSetPosVelVar")) {
		float pos_var;
		if((nrhs != 2) || !mlGetFloatArray(prhs[1], &pos_var, 1)) {
			mexErrMsgTxt("Error with input parameters\n");
			return;
		}
		INSSetPosVelVar(pos_var);
	} else if (mlStringCompare(prhs[0], "INSSetGyroBias")) {
		float gyro_bias[3];
		if((nrhs != 2) || !mlGetFloatArray(prhs[1], gyro_bias, 3)) {
			mexErrMsgTxt("Error with input parameters\n");
			return;
		}
		INSSetGyroBias(gyro_bias);
	} else if (mlStringCompare(prhs[0], "INSSetAccelVar")) {
		float accel_var[3];
		if((nrhs != 2) || !mlGetFloatArray(prhs[1], accel_var, 3)) {
			mexErrMsgTxt("Error with input parameters\n");
			return;
		}
		INSSetAccelVar(accel_var);
	} else if (mlStringCompare(prhs[0], "INSSetGyroVar")) {
		float gyro_var[3];
		if((nrhs != 2) || !mlGetFloatArray(prhs[1], gyro_var, 3)) {
			mexErrMsgTxt("Error with input parameters\n");
			return;
		}
		INSSetGyroVar(gyro_var);
	} else if (mlStringCompare(prhs[0], "INSSetMagNorth")) {
		float mag_north[3];
		float Bmag;
		if((nrhs != 2) || !mlGetFloatArray(prhs[1], mag_north, 3)) {
			mexErrMsgTxt("Error with input parameters\n");
			return;
		}
		Bmag = sqrt(mag_north[0] * mag_north[0] + mag_north[1] * mag_north[1] +
				mag_north[2] * mag_north[2]);
		mag_north[0] = mag_north[0] / Bmag;
		mag_north[1] = mag_north[1] / Bmag;
		mag_north[2] = mag_north[2] / Bmag;

		INSSetMagNorth(mag_north);
	} else if (mlStringCompare(prhs[0], "INSSetMagVar")) {
		float mag_var[3];
		if((nrhs != 2) || !mlGetFloatArray(prhs[1], mag_var, 3)) {
			mexErrMsgTxt("Error with input parameters\n");
			return;
		}
		INSSetMagVar(mag_var);
	} else if (mlStringCompare(prhs[0], "INSSetState")) {
        int i;
		float new_state[NUMX];
		if((nrhs != 2) || !mlGetFloatArray(prhs[1], new_state, NUMX)) {
			mexErrMsgTxt("Error with input parameters\n");
			return;
		}
		for(i = 0; i < NUMX; i++)
			X[i] = new_state[i];
	} else {
		mexErrMsgTxt("Unknown function");
	}

	if(nlhs > 0) {
		// return current state vector
		double * data_out;
		int i;

		plhs[0] = mxCreateDoubleMatrix(1,13,0);
		data_out = mxGetData(plhs[0]);
		for(i = 0; i < NUMX; i++)
			data_out[i] = X[i];
	}

	if(nlhs > 1) {
		//return covariance estimate
		double * data_copy = mxCalloc(NUMX*NUMX, sizeof(double));
		int i, j, k;

		plhs[1] = mxCreateDoubleMatrix(13,13,0);
		for(i = 0; i < NUMX; i++)
			for(j = 0; j < NUMX; j++)
			{
				data_copy[j + i * NUMX] = P[j][i];
			}

		mxSetData(plhs[1], data_copy);
	}

	if(nlhs > 2) {
		//return covariance estimate
		double * data_copy = mxCalloc(NUMX*NUMV, sizeof(double));
		int i, j, k;

		plhs[2] = mxCreateDoubleMatrix(NUMX,NUMV,0);
		for(i = 0; i < NUMX; i++)
			for(j = 0; j < NUMV; j++)
			{
				data_copy[j + i * NUMX] = K[i][j];
			}

		mxSetData(plhs[2], data_copy);
	}
	return;
}
        
bool mlGetFloatArray(const mxArray * mlVal, float * dest, int numel) {
	if(!mxIsNumeric(mlVal) || (!mxIsDouble(mlVal) && !mxIsSingle(mlVal)) || (mxGetNumberOfElements(mlVal) != numel)) {
		mexErrMsgTxt("Data misformatted (either not double or not the right number)");
		return false;
	}
	
	if(mxIsSingle(mlVal)) {
		memcpy(dest,mxGetData(mlVal),numel*sizeof(*dest));
	} else {
		int i;
		double * data_in = mxGetData(mlVal);
		for(i = 0; i < numel; i++)
			dest[i] = data_in[i]; 
	}

	return true;
}

bool mlStringCompare(const mxArray * mlVal, char * cStr) {
	int i;
	char * mlCStr = 0;
	bool val = false;
	int strLen = mxGetNumberOfElements(mlVal);

	mlCStr = mxCalloc((1+strLen),  sizeof(*mlCStr));
	if(!mlCStr)
		return false;

	if(mxGetString(mlVal, mlCStr, strLen+1))
		goto cleanup;

	for(i = 0; i < strLen; i++) {
		if(mlCStr[i] != cStr[i]) 
			goto cleanup;
	}

	if(cStr[i] == '\0')
		val = true;
	
cleanup:
	if(mlCStr) {
		mxFree(mlCStr);
		mlCStr = 0;
	}
	return val;
}
