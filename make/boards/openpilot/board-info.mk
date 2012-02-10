BOARD_TYPE          := 0x01
BOARD_REVISION      := 0x01
BOOTLOADER_VERSION  := 0x01
HW_TYPE             := 0x00

MCU                 := cortex-m3
CHIP                := STM32F103RET
BOARD               := STM3210E_OP
MODEL               := HD
MODEL_SUFFIX        := _OP

OPENOCD_CONFIG      := stm32f1x.cfg

# Note: These must match the values in link_$(BOARD)_memory.ld
BL_BANK_BASE        := 0x08000000  # Start of bootloader flash
BL_BANK_SIZE        := 0x00005000  # Should include BD_INFO region
FW_BANK_BASE        := 0x08005000  # Start of firmware flash
FW_BANK_SIZE        := 0x0007B000  # Should include FW_DESC_SIZE

FW_DESC_SIZE        := 0x00000064
