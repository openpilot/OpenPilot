#include "ahrs_bl.h"
#include "ahrs_spi_program.h"

uint8_t buf[256];

bool StartProgramming(void) {
	PIOS_COM_SendFormattedString(PIOS_COM_AUX, "Started programming\r\n");
	return (true);
}

bool WriteData(uint32_t offset, uint8_t *buffer, uint32_t size) {
	if (size > SPI_MAX_PROGRAM_DATA_SIZE) {
		PIOS_COM_SendFormattedString(PIOS_COM_AUX, "oversize: %d\r\n", size);
		return (false);
	}
	PIOS_COM_SendFormattedString(PIOS_COM_AUX, "Wrote %d bytes to %d\r\n",
			size, offset);
	memcpy(buf, buffer, size);
	PIOS_LED_Toggle(LED1);
	return (true);
}

bool ReadData(uint32_t offset, uint8_t *buffer, uint32_t size) {
	if (size > SPI_MAX_PROGRAM_DATA_SIZE) {
		PIOS_COM_SendFormattedString(PIOS_COM_AUX, "oversize: %d\r\n", size);
		return (false);
	}
	PIOS_COM_SendFormattedString(PIOS_COM_AUX, "Read %d bytes from %d\r\n",
			size, offset);
	memcpy(buffer, buf, size);
	PIOS_LED_Toggle(LED1);
	return (true);
}

