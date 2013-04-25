
#include "pios.h"
#include "pios_sim_priv.h"
#include "sim_model.h"

struct pios_sim_state pios_sim_state = {
	.accels = {0, 0, 0},
	.gyros = {0, 0, 0},
	.mag = {0, 0, 0},
	.baro = {0},
	.q = {1, 0, 0, 0},
	.velocity = {0, 0, 0},
	.position = {0, 0, 0},	
	.actuator = {0, 0, 0, 0, 0, 0, 0, 0}
};

/**
 * Initialize the model in the external library
 * @returns 0 for success, -1 if fails to initialize external library
 */
int PIOS_SIM_Init() 
{
	if (sim_model_init() != 0)
		return -1;
	return 0;
}

/**
 * Step the model simulation in the external library
 * @returns 0 for success, -1 for failure to step external library
 */
int PIOS_SIM_Step(float dT) 
{
	if (sim_model_step(dT, &pios_sim_state) != 0)
		return -1;

	return 0;
}

/**
 * Set the actuator inputs to the model
 * @param[in] actuator pointer to an array of actuators to set
 * @param[in] nchannels number of channels that are valid coming in
 */
void PIOS_SIM_SetActuator(float * actuator, int nchannels)
{
	for (int i = 0; i < NELEMENTS(pios_sim_state.actuator) && i < nchannels; i++)
		pios_sim_state.actuator[i] = actuator[i];
}

/**
 * Get the accelerometer data from the simulation model
 * @param[out] pointer to store the accelerometer data in
 */
void PIOS_SIM_GetAccels(float * accels)
{
	for (int i = 0; i < NELEMENTS(pios_sim_state.accels); i++)
		accels[i] = pios_sim_state.accels[i];
}

/**
 * Get the gyro data from the simulation model
 * @param[out] pointer to store the gyro data in
 */
void PIOS_SIM_GetGyros(float * gyros)
{
	for (int i = 0; i < NELEMENTS(pios_sim_state.accels); i++)
		gyros[i] = pios_sim_state.gyros[i];
}

/**
 * Get the current attitude from the simulation model
 * @param[out] quat pointer to store the quaternion attitude in
 */
void PIOS_SIM_GetAttitude(float * q)
{
	for (int i = 0; i < NELEMENTS(pios_sim_state.q); i++)
		q[i] = pios_sim_state.q[i];
}

/**
 * Get the current positiom from the simulation model
 * @param[out] position pointer to store the current position in (cm in NED
 * frame)
 */
void PIOS_SIM_GetVelocity(float * velocity)
{
	for (int i = 0; i < NELEMENTS(pios_sim_state.velocity); i++)
		velocity[i] = pios_sim_state.velocity[i];
}

/**
 * Get the current positiom from the simulation model
 * @param[out] position pointer to store the current position in (cm in NED
 * frame)
 */
void PIOS_SIM_GetPosition(float * position)
{
	for (int i = 0; i < NELEMENTS(pios_sim_state.position); i++)
		position[i] = pios_sim_state.position[i];
}
