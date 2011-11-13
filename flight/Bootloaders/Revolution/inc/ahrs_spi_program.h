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
 packetId should correspond to the transmitted packet.
 4) repeat for next packet until finished
 5) Repeat using verify packets
 Returned data should be exactly as read from memory

 In the slave:
 1) Wait for an AhrsProgramPacket
 2) Check CRC then write to memory
 3) Bitwise invert data and add it to the return packet
 4) Copy packetId from received packet
 5) Transmit packet.
 6) repeat until we receive a read packet
 7) read memory as requested until we receive a reboot packet
 Reboot packets had better have some sort of magic number in the data,
 just to be absolutely sure

 */

typedef enum {
	PROGRAM_NULL,
	PROGRAM_WRITE,
	PROGRAM_READ,
	PROGRAM_ACK,
	PROGRAM_REBOOT,
	PROGRAM_ERR
} ProgramType;
#define SPI_MAX_PROGRAM_DATA_SIZE (14 * 4)	//USB comms uses 14x 32 bit words
#define REBOOT_CONFIRMATION "AHRS REBOOT"
#define REBOOT_CONFIRMATION_LENGTH 11

/** Proposed program packet defintion
 */

typedef struct {
	ProgramType type;
	uint32_t packetId; //Transmission packet ID
	uint32_t address; //base address to place data
	uint32_t size; //Size of data (0 to SPI_MAX_PROGRAM_DATA_SIZE)
	uint8_t data[SPI_MAX_PROGRAM_DATA_SIZE];
	uint32_t crc; //CRC32 - hardware CRC8 can be fooled
	uint8_t dummy; //for some reason comms trashes the last byte sent
} AhrsProgramPacket;

uint32_t GenerateCRC(AhrsProgramPacket * packet);

#endif
