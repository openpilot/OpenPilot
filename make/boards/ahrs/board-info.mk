BOARD_TYPE          := 0x02
BOARD_REVISION      := 0x01
BOOTLOADER_VERSION  := 0x00
HW_TYPE             := 0x00

MCU                 := cortex-m3
CHIP                := STM32F103CBT
BOARD               := STM32103CB_AHRS
MODEL               := MD
MODEL_SUFFIX        := 

OPENOCD_CONFIG      := stm32f1x.cfg

# Note: These must match the values in link_$(BOARD)_memory.ld
BL_BANK_BASE        := 0x08000000  # Start of bootloader flash
BL_BANK_SIZE        := 0x00002000  # Should include BD_INFO region
FW_BANK_BASE        := 0x08002000  # Start of firmware flash
FW_BANK_SIZE        := 0x0001E000  # Should include FW_DESC_SIZE

FW_DESC_SIZE        := 0x00000064
