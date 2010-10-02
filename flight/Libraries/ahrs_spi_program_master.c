#include "ahrs_program_master.h"
#include "ahrs_program.h"
#include "pios_spi.h"

char connectTxBuf[SPI_PROGRAM_REQUEST_LENGTH] = { SPI_PROGRAM_REQUEST };

char connectRxBuf[SPI_PROGRAM_REQUEST_LENGTH];
#define MAX_CONNECT_TRIES 10

uint32_t AhrsProgramConnect(void)
{
	memset(connectRxBuf, 0, SPI_PROGRAM_REQUEST_LENGTH);
	for (int ct = 0; ct < MAX_CONNECT_TRIES; ct++) {
		uint32_t res = PIOS_SPI_TransferBlock(PIOS_SPI_OP, (uint8_t *) & connectTxBuf,
						      (uint8_t *) & connectRxBuf, SPI_PROGRAM_REQUEST_LENGTH, NULL);
		if (res == 0 && memcmp(connectRxBuf, SPI_PROGRAM_ACK, SPI_PROGRAM_REQUEST_LENGTH) == 0) {
			return (0);
		}

		vTaskDelay(1 / portTICK_RATE_MS);
	}
	return (-1);
}
