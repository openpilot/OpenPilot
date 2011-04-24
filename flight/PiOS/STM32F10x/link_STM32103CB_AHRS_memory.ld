MEMORY
{
    BL_FLASH (rx) : ORIGIN = 0x08000000,      LENGTH =   8K
    FLASH (rx)    : ORIGIN = 0x08000000 + 8K, LENGTH = 128K - 8K
    SRAM (rwx)    : ORIGIN = 0x20000000,      LENGTH =  20K
}
