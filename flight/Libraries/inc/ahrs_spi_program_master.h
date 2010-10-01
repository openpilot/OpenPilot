#ifndef AHRS_PROGRAM_MASTER_H
#define AHRS_PROGRAM_MASTER_H

/** Connect to AHRS and request programming mode
* returns: 0 if connected, -1 if failed.
*/
uint32_t AhrsProgramConnect(void);

//TODO: Implement programming protocol

#endif //AHRS_PROGRAM_MASTER_H
