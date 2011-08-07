/* XXX should be based on the SRAM definition */
_estack = 0x2001fff0;

/* Section Definitions */
SECTIONS
{
    .text :
    {
        PROVIDE (pios_isr_vector_table_base = .);
        KEEP(*(.isr_vector .isr_vector.*))
        *(.text .text.* .gnu.linkonce.t.*)
        *(.glue_7t) *(.glue_7)
        *(.rodata .rodata* .gnu.linkonce.r.*)
    } > BL_FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH

    . = ALIGN(4);
    _etext = .;
    _sidata = .;

    .data : AT (_etext)
    {
        _sdata = .;
        *(.data .data.*)
        . = ALIGN(4);
        _edata = . ;
    } > SRAM

    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {
        _sbss = . ;
        *(.bss .bss.*)
        *(COMMON)
        _ebss = . ;
        
    } > SRAM

    . = ALIGN(4);
    _end = . ;

    .boardinfo :
    {
		. = ALIGN(4);
		KEEP(*(.boardinfo))
	    } > BD_INFO

}
