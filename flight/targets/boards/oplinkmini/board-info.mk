BOARD_TYPE          := 0x03
BOARD_REVISION      := 0x01
BOOTLOADER_VERSION  := 0x04
HW_TYPE             := 0x01

MCU                 := cortex-m3
CHIP                := STM32F103CBT
BOARD               := STM32103CB_OPLINKMINI
MODEL               := MD
MODEL_SUFFIX        := _PX

OPENOCD_JTAG_CONFIG := stlink-v2.cfg
OPENOCD_CONFIG      := stm32f1x.stlink.cfg

# Flash memory map for OPLM:
# Sector	start			size	use
# 0			0x0800 0000		1k		BL
# 1			0x0800 0400		1k		BL
# ..								..
# 10		0x0800 2C00		1k		BL
# 11		0x0800 3000		1k		FW
# 12		0x0800 1000		1k		FW
# ..								..
# 125 		0x0801 F400		1k		FW
# 126 		0x0801 F800		1k		EE
# 127 		0x0801 FC00		1k		EE


# Note: These must match the values in link_$(BOARD)_memory.ld
BL_BANK_BASE        := 0x08000000  # Start of bootloader flash
BL_BANK_SIZE        := 0x00003000  # Should include BD_INFO region
FW_BANK_BASE        := 0x08003000  # Start of firmware flash
FW_BANK_SIZE        := 0x0001CC00  # Should include FW_DESC_SIZE
FW_BANK_SIZE        := 0x00019000  # Should include FW_DESC_SIZE
EE_BANK_BASE        := 0x0801C000  # EEPROM storage area
EE_BANK_SIZE        := 0x00004000  # Size of EEPROM storage area

FW_DESC_SIZE        := 0x00000064
