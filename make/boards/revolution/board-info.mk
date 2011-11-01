BOARD_TYPE          := 0x05
BOARD_REVISION      := 0x01
BOOTLOADER_VERSION  := 0x00
HW_TYPE             := 0x00

MCU                 := cortex-m4
CHIP                := STM32F205RGT
BOARD               := STM32F2xx_INS
MODEL               := HD
MODEL_SUFFIX        := 

OPENOCD_CONFIG      := stm32f4xx.cfg

# Note: These must match the values in link_$(BOARD)_memory.ld
BL_BANK_BASE        := 0x08000000  # Start of bootloader flash
BL_BANK_SIZE        := 0x00008000  # Should include BD_INFO region
FW_BANK_BASE        := 0x08008000  # Start of firmware flash
FW_BANK_SIZE        := 0x0001E000  # Should include FW_DESC_SIZE

FW_DESC_SIZE        := 0x00000064

OSCILLATOR_FREQ     := 8000000
SYSCLK_FREQ         := 168000000
