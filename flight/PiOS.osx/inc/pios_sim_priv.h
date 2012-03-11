
#ifndef PIOS_SIM_PRIV_H
#define PIOS_SIM_PRIV_H

/**
 * State of inputs and outputs to the simulation model
 */
struct pios_sim_state {
	float accels[3];
	float gyros[3];
	float mag[3];
	float baro[1];
	float q[4];
	float velocity[3];
	float position[3];
	float actuator[8];
};

#endif /* PIOS_SIM_PRIV */
