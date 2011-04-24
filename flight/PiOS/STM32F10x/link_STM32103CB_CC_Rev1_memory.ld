MEMORY
{
    BL_FLASH (rx) : ORIGIN = 0x08000000,       LENGTH =  12K
    FLASH (rx)    : ORIGIN = 0x08000000 + 12K, LENGTH = 128K - 12K
    SRAM (rwx)    : ORIGIN = 0x20000000,       LENGTH =  20K
}
