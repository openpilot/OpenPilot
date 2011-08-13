MEMORY
{
    BL_FLASH (rx) : ORIGIN = 0x08000000,        LENGTH = 0x08000 -  0x00080
    BD_INFO (r)   : ORIGIN = 0x08008000 - 0x80, LENGTH =            0x00080
    FLASH (rx)    : ORIGIN = 0x08008000,        LENGTH = 0x100000 - 0x08000
    SRAM (rwx)    : ORIGIN = 0x20000000,        LENGTH = 0x05000
}
