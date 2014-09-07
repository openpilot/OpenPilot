MEMORY
{
    BL_FLASH (rx) : ORIGIN = 0x08000000,                LENGTH = 10K - 0x80
    BD_INFO (r)   : ORIGIN = 0x08000000 + 10K - 0x80,    LENGTH = 0x80
    FLASH (rx)    : ORIGIN = 0x08000000 + 10K,           LENGTH = 32K - 10K
    VTRAM (xrw)   : ORIGIN = 0x20000000, LENGTH = 0xc0  /* First part of RAM is reserved to the vector table */
    SRAM (rwx)    : ORIGIN = 0x20000000 + 0xc0,                LENGTH = 4K - 0xc0
}
