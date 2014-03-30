MEMORY
{
/*    rom (rx)  : ORIGIN = 0x08000000, LENGTH = 32K */
/*    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 4K  */
    BL_FLASH (rx) : ORIGIN = 0x08000000,                LENGTH = 2K - 0x80
    BD_INFO (r)   : ORIGIN = 0x08000000 + 2K - 0x80,    LENGTH = 0x80
    FLASH (rx)    : ORIGIN = 0x08000000 + 2K,           LENGTH = 32K - 2K
    SRAM (rwx)    : ORIGIN = 0x20000000,                LENGTH = 4K
}
