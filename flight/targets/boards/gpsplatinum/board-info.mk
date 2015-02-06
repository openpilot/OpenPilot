BOARD_TYPE          := 0x0A
BOARD_REVISION      := 0x01
BOOTLOADER_VERSION  := 0x01
HW_TYPE             := 0x01

MCU                 := cortex-m0
CHIP                := STM32F051G
BOARD               := STM3205x_GPSP
MODEL               := 51
MODEL_SUFFIX        := _gpsp

OPENOCD_JTAG_CONFIG := foss-jtag.revb.cfg
OPENOCD_CONFIG      := stm32f0x.cfg

# Note: These must match the values in link_$(BOARD)_memory.ld
BL_BANK_BASE        := 0x08000000  # Start of bootloader flash
BL_BANK_SIZE        := 0x00002800  # Should include BD_INFO region
FW_BANK_BASE        := 0x08002800  # Start of firmware flash
FW_BANK_SIZE        := 0x00005800  # Should include FW_DESC_SIZE

FW_DESC_SIZE        := 0x00000064

OSCILLATOR_FREQ     :=   8000000
