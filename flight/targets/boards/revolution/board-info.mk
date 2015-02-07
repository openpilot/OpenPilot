BOARD_TYPE          := 0x09
BOARD_REVISION      := 0x03
BOOTLOADER_VERSION  := 0x06
HW_TYPE             := 0x00

MCU                 := cortex-m4
CHIP                := STM32F405RGT
BOARD               := STM32F4xx_RM
MODEL               := HD
MODEL_SUFFIX        := 

OPENOCD_JTAG_CONFIG := stlink-v2.cfg
OPENOCD_CONFIG      := stm32f4xx.stlink.cfg

USE_SPIFFS          := YES
# Flash memory map for Revolution:
# Sector	start		size	use
# 0		0x0800 0000	16k	BL
# 1		0x0800 4000	16k	BL
# 2		0x0800 8000	16k	EE
# 3		0x0800 C000	16k	EE
# 4		0x0801 0000	64k	Unused
# 5		0x0802 0000	128k	FW
# 6		0x0804 0000	128k	FW
# 7 		0x0806 0000	128k	FW
# 8 		0x0808 0000	128k	Unused
# 9		0x080A 0000	128k	Unused
# 10		0x080C 0000	128k	Unused						..
# 11		0x080E 0000	128k   	Unused

# Note: These must match the values in link_$(BOARD)_memory.ld
BL_BANK_BASE        := 0x08000000  # Start of bootloader flash
BL_BANK_SIZE        := 0x00008000  # Should include BD_INFO region

# 16KB for settings storage

EE_BANK_BASE        := 0x08008000  # EEPROM storage area
EE_BANK_SIZE        := 0x00008000  # Size of EEPROM storage area

# Leave the remaining 64KB sectors for other uses

FW_BANK_BASE        := 0x08020000  # Start of firmware flash
FW_BANK_SIZE        := 0x000A0000  # Should include FW_DESC_SIZE

FW_DESC_SIZE        := 0x00000064

OSCILLATOR_FREQ     :=   8000000
SYSCLK_FREQ         := 168000000
