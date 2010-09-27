#ifndef NMEA_H
#define NMEA_H

#include <stdbool.h>
#include <stdint.h>

extern bool NMEA_update_position(char *nmea_sentence);
extern bool NMEA_checksum(char *nmea_sentence);

#endif /* NMEA_H */
