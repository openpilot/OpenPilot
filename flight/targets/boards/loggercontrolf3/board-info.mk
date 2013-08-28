BOARD_TYPE          := 0x09
BOARD_REVISION      := 0x03
BOOTLOADER_VERSION  := 0x05
HW_TYPE             := 0x00

MCU                 := cortex-m4
CHIP                := STM32F303CCT
BOARD               := STM32F3xx_RM
MODEL               := HD
MODEL_SUFFIX        := 

OPENOCD_JTAG_CONFIG := stlink-v2.cfg
OPENOCD_CONFIG      := stm32f1x.cfg

# Flash memory map for Revolution:
# Sector	start		size	use
# 0		0x0800 0000	16k	BL
# 2		0x0800 4000	16k	EE
# 3		0x0800 8000	16k	EE
# 4		0x0800 c000	64k	Unused
# 5		0x0801 c000	128k	FW
# 6		0x0803 c000	128k	FW

# Note: These must match the values in link_$(BOARD)_memory.ld
BL_BANK_BASE        := 0x08000000  # Start of bootloader flash
BL_BANK_SIZE        := 0x00004000  # Should include BD_INFO region (16kB)

EE_BANK_BASE        := 0x08004000  # @16kB
EE_BANK_SIZE        := 0x00008000  # (32kb)

# Leave the remaining 16KB and 64KB sectors for other uses
FW_BANK_BASE        := 0x0800C000  # Start of firmware flash @48kB
FW_BANK_SIZE        := 0x00034000  # Should include FW_DESC_SIZE (208kB)

FW_DESC_SIZE        := 0x00000064

OSCILLATOR_FREQ     :=   8000000
SYSCLK_FREQ         :=  72000000
