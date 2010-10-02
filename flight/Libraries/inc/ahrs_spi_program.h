#ifndef AHRS_SPI_PROGRAM_H
#define AHRS_SPI_PROGRAM_H

/* Special packets to enter programming mode.
Note: these must both be SPI_PROGRAM_REQUEST_LENGTH long.
Pad with spaces if needed.
*/
#define SPI_PROGRAM_REQUEST "AHRS START PROGRAMMING  "
#define SPI_PROGRAM_ACK     "AHRS PROGRAMMING STARTED"
#define SPI_PROGRAM_REQUEST_LENGTH 24

/**Proposed programming protocol:
In the master:
1) Send a AhrsProgramPacket containing the relevant data.
Note crc is a CRC32 as the CRC8 used in hardware can be fooled.
2) Keep sending PROGRAM_NULL packets and wait for an ack.
   Time out if we waited too long.
3) Compare ack packet with transmitted packet. The data
   should be the bitwise inverse of the data transmitted.
4) repeat for next packet until finished
5) Repeat using verify packets with data all zeros
   Returned data should be exactly as read from memory

In the slave:
1) Wait for an AhrsProgramPacket
2) Check CRC then write to memory
3) Bitwise invert data
4) Transmit packet.
5) repeat until we receive a verify packet
6) verify until we receive a reboot packet
Reboot packets had better have some sort of magic number in the data,
just to be absolutely sure

*/

typedef enum { PROGRAM_NULL, PROGRAM_DATA, PROGRAM_ACK, PROGRAM_VERIFY, PROGRAM_REBOOT } ProgramType;
#define SPI_MAX_PROGRAM_DATA_SIZE 256	//max 256 bytes per packet

/** Proposed program packet defintion
*/

typedef struct {
	ProgramType type;
	uint32_t address;	//base address to place data
	uint32_t size;		//Size of data
	uint8_t data[SPI_MAX_PROGRAM_DATA_SIZE];
	uint32_t crc;		//CRC32 - hardware CRC8 can be fooled
} AhrsProgramPacket;

#endif
