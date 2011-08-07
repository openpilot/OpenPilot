/* This is the size of the stack for all FreeRTOS IRQs */
_irq_stack_size = 0x400;
/* This is the size of the stack for early init: life span is until scheduler starts */
_init_stack_size = 0x400;


PROVIDE(pios_board_info_blob = ORIGIN(BD_INFO));

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
    } > FLASH

    /* init sections */
    .initcalluavobj.init :
    {
        . = ALIGN(4);
	__uavobj_initcall_start = .;
        KEEP(*(.initcalluavobj.init))
	. = ALIGN(4);
	__uavobj_initcall_end   = .;
    } >FLASH

    /* module sections */
    .initcallmodule.init :
    {
        . = ALIGN(4);
	__module_initcall_start = .;
        KEEP(*(.initcallmodule.init))
	. = ALIGN(4);
	__module_initcall_end   = .;
    } >FLASH

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

    /*
     * This stack is used both as the initial sp during early init as well as ultimately
     * being used as the STM32's MSP (Main Stack Pointer) which is the same stack that
     * is used for _all_ interrupt handlers.  The end of this stack should be placed
     * against the lowest address in RAM so that a stack overrun results in a hard fault
     * at the first access beyond the end of the stack.
     */
    .irq_stack :
    {
         . = ALIGN(4);
         _irq_stack_end = . ;
         . = . + _irq_stack_size ;
         . = ALIGN(4);
         _irq_stack_top = . - 4 ;
         . = ALIGN(4);
    } > SRAM

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

    .heap (NOLOAD) :
    {
        . = ALIGN(4);
        _heap = ABSOLUTE(.);
        _sheap = . ;
        *(.heap)
        
        /* fill to the end of SRAM */
        . = ORIGIN(SRAM) + LENGTH(SRAM) - _heap ;
        . = ALIGN(4) ;
        
        /* place the init stack at the end of SRAM */
        _init_stack_top = . - 4 ;
        _init_stack_end = . - _init_stack_size ;
        
        /* extend the heap up to the base of the init stack */
        _eheap = _init_stack_end ;
    } > SRAM

    .free_ram (NOLOAD) :
    {
		_eram = . ;
    } > SRAM

    /* keep the heap section at the end of the SRAM
     * this will allow to claim the remaining bytes not used
     * at run time! (done by the reset vector).
     */

    PROVIDE ( _end = _ebss ) ;

    /* Stabs debugging sections.  */
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    /* DWARF debug sections.
       Symbols in the DWARF debugging sections are relative to the beginning
       of the section so we begin them at 0.  */
    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
}
