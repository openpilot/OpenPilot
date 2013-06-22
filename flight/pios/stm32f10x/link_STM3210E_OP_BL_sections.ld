/* This is the size of the stack for early init and for all FreeRTOS IRQs */
_irq_stack_size = 0x800;

/* Check valid alignment for VTOR */
ASSERT(ORIGIN(BL_FLASH) == ALIGN(ORIGIN(BL_FLASH), 0x80), "Start of memory region flash not aligned for startup vector table");

/*
this sends all unreferenced IRQHandlers to reset
*/


PROVIDE (   Undefined_Handler = 0 ) ;
PROVIDE (   SWI_Handler = 0 ) ;
PROVIDE (   IRQ_Handler = 0 ) ;
PROVIDE (   Prefetch_Handler = 0 ) ;
PROVIDE (   Abort_Handler = 0 ) ;
PROVIDE (   FIQ_Handler = 0 ) ;

PROVIDE (   NMI_Handler = 0 ) ;
PROVIDE (   HardFault_Handler = 0 ) ;
PROVIDE (   MemManage_Handler = 0 ) ;
PROVIDE (   BusFault_Handler = 0 ) ;
PROVIDE (   UsageFault_Handler = 0 ) ;
PROVIDE (   vPortSVCHandler = 0 ) ;
PROVIDE (   DebugMon_Handler = 0 ) ;
PROVIDE (   xPortPendSVHandler = 0 ) ;
PROVIDE (   xPortSysTickHandler = 0 ) ;

PROVIDE (   WWDG_IRQHandler = 0 ) ;
PROVIDE (   PVD_IRQHandler = 0 ) ;
PROVIDE (   TAMPER_IRQHandler = 0 ) ;
PROVIDE (   RTC_IRQHandler = 0 ) ;
PROVIDE (   FLASH_IRQHandler = 0 ) ;
PROVIDE (   RCC_IRQHandler = 0 ) ;
PROVIDE (   EXTI0_IRQHandler = 0 ) ;
PROVIDE (   EXTI1_IRQHandler = 0 ) ;
PROVIDE (   EXTI2_IRQHandler = 0 ) ;
PROVIDE (   EXTI3_IRQHandler = 0 ) ;
PROVIDE (   EXTI4_IRQHandler = 0 ) ;
PROVIDE (   DMAChannel1_IRQHandler = 0 ) ;
PROVIDE (   DMAChannel2_IRQHandler = 0 ) ;
PROVIDE (   DMAChannel3_IRQHandler = 0 ) ;
PROVIDE (   DMAChannel4_IRQHandler = 0 ) ;
PROVIDE (   DMAChannel5_IRQHandler = 0 ) ;
PROVIDE (   DMAChannel6_IRQHandler = 0 ) ;
PROVIDE (   DMAChannel7_IRQHandler = 0 ) ;
PROVIDE (   ADC_IRQHandler = 0 ) ;
PROVIDE (   USB_HP_CAN1_TX_IRQHandler = 0 ) ;
PROVIDE (   USB_LP_CAN1_RX0_IRQHandler = 0 ) ;
PROVIDE (   CAN1_RX1_IRQHandler = 0 ) ;
PROVIDE (   CAN1_SCE_IRQHandler = 0 ) ;
PROVIDE (   EXTI9_5_IRQHandler = 0 ) ;
PROVIDE (   TIM1_BRK_IRQHandler = 0 ) ;
PROVIDE (   TIM1_UP_IRQHandler = 0 ) ;
PROVIDE (   TIM1_TRG_COM_IRQHandler = 0 ) ;
PROVIDE (   TIM1_CC_IRQHandler = 0 ) ;
PROVIDE (   TIM2_IRQHandler = 0 ) ;
PROVIDE (   TIM3_IRQHandler = 0 ) ;
PROVIDE (   TIM4_IRQHandler = 0 ) ;
PROVIDE (   I2C1_EV_IRQHandler = 0 ) ;
PROVIDE (   I2C1_ER_IRQHandler = 0 ) ;
PROVIDE (   I2C2_EV_IRQHandler = 0 ) ;
PROVIDE (   I2C2_ER_IRQHandler = 0 ) ;
PROVIDE (   SPI1_IRQHandler = 0 ) ;
PROVIDE (   SPI2_IRQHandler = 0 ) ;
PROVIDE (   USART1_IRQHandler = 0 ) ;
PROVIDE (   USART2_IRQHandler = 0 ) ;
PROVIDE (   USART3_IRQHandler = 0 ) ;
PROVIDE (   EXTI15_10_IRQHandler = 0 ) ;
PROVIDE (   RTCAlarm_IRQHandler = 0 ) ;
PROVIDE (   USBWakeUp_IRQHandler = 0 ) ;
PROVIDE (   TIM8_BRK_IRQHandler = 0 ) ;
PROVIDE (   TIM8_UP_IRQHandler = 0 ) ;
PROVIDE (   TIM8_TRG_COM_IRQHandler = 0 ) ;
PROVIDE (   TIM8_CC_IRQHandler = 0 ) ;
PROVIDE (   ADC3_IRQHandler = 0 ) ;
PROVIDE (   FSMC_IRQHandler = 0 ) ;
PROVIDE (   SDIO_IRQHandler = 0 ) ;
PROVIDE (   TIM5_IRQHandler = 0 ) ;
PROVIDE (   SPI3_IRQHandler = 0 ) ;
PROVIDE (   UART4_IRQHandler = 0 ) ;
PROVIDE (   UART5_IRQHandler = 0 ) ;
PROVIDE (   TIM6_IRQHandler = 0 ) ;
PROVIDE (   TIM7_IRQHandler = 0 ) ;
PROVIDE (   DMA2_Channel1_IRQHandler = 0 ) ;
PROVIDE (   DMA2_Channel2_IRQHandler = 0 ) ;
PROVIDE (   DMA2_Channel3_IRQHandler = 0 ) ;
PROVIDE (   DMA2_Channel4_5_IRQHandler = 0 ) ;



/******************************************************************************/
/*                       Peripheral memory map                                */
/******************************************************************************/
/*this allows to compile the ST lib in "non-debug" mode*/


/* Peripheral and SRAM base address in the alias region */
PERIPH_BB_BASE        = 0x42000000;
SRAM_BB_BASE          = 0x22000000;

/* Peripheral and SRAM base address in the bit-band region */
SRAM_BASE             = 0x20000000;
PERIPH_BASE           = 0x40000000;

/* Flash registers base address */
PROVIDE ( FLASH_BASE            = 0x40022000);
/* Flash Option Bytes base address */
PROVIDE ( OB_BASE               = 0x1FFFF800);

/* Peripheral memory map */
APB1PERIPH_BASE      = PERIPH_BASE  ;
APB2PERIPH_BASE      = (PERIPH_BASE + 0x10000) ;
AHBPERIPH_BASE       = (PERIPH_BASE + 0x20000) ;

PROVIDE ( TIM2            = (APB1PERIPH_BASE + 0x0000) ) ;
PROVIDE ( TIM3            = (APB1PERIPH_BASE + 0x0400) ) ;
PROVIDE ( TIM4            = (APB1PERIPH_BASE + 0x0800) ) ;
PROVIDE ( RTC             = (APB1PERIPH_BASE + 0x2800) ) ;
PROVIDE ( WWDG            = (APB1PERIPH_BASE + 0x2C00) ) ;
PROVIDE ( IWDG            = (APB1PERIPH_BASE + 0x3000) ) ;
PROVIDE ( SPI2            = (APB1PERIPH_BASE + 0x3800) ) ;
PROVIDE ( USART2          = (APB1PERIPH_BASE + 0x4400) ) ;
PROVIDE ( USART3          = (APB1PERIPH_BASE + 0x4800) ) ;
PROVIDE ( I2C1            = (APB1PERIPH_BASE + 0x5400) ) ;
PROVIDE ( I2C2            = (APB1PERIPH_BASE + 0x5800) ) ;
PROVIDE ( CAN             = (APB1PERIPH_BASE + 0x6400) ) ;
PROVIDE ( BKP             = (APB1PERIPH_BASE + 0x6C00) ) ;
PROVIDE ( PWR             = (APB1PERIPH_BASE + 0x7000) ) ;

PROVIDE ( AFIO            = (APB2PERIPH_BASE + 0x0000) ) ;
PROVIDE ( EXTI            = (APB2PERIPH_BASE + 0x0400) ) ;
PROVIDE ( GPIOA           = (APB2PERIPH_BASE + 0x0800) ) ;
PROVIDE ( GPIOB           = (APB2PERIPH_BASE + 0x0C00) ) ;
PROVIDE ( GPIOC           = (APB2PERIPH_BASE + 0x1000) ) ;
PROVIDE ( GPIOD           = (APB2PERIPH_BASE + 0x1400) ) ;
PROVIDE ( GPIOE           = (APB2PERIPH_BASE + 0x1800) ) ;
PROVIDE ( ADC1            = (APB2PERIPH_BASE + 0x2400) ) ;
PROVIDE ( ADC2            = (APB2PERIPH_BASE + 0x2800) ) ;
PROVIDE ( TIM1            = (APB2PERIPH_BASE + 0x2C00) ) ;
PROVIDE ( SPI1            = (APB2PERIPH_BASE + 0x3000) ) ;
PROVIDE ( USART1          = (APB2PERIPH_BASE + 0x3800) ) ;

PROVIDE ( DMA             = (AHBPERIPH_BASE + 0x0000) ) ;
PROVIDE ( DMA_Channel1    = (AHBPERIPH_BASE + 0x0008) ) ;
PROVIDE ( DMA_Channel2    = (AHBPERIPH_BASE + 0x001C) ) ;
PROVIDE ( DMA_Channel3    = (AHBPERIPH_BASE + 0x0030) ) ;
PROVIDE ( DMA_Channel4    = (AHBPERIPH_BASE + 0x0044) ) ;
PROVIDE ( DMA_Channel5    = (AHBPERIPH_BASE + 0x0058) ) ;
PROVIDE ( DMA_Channel6    = (AHBPERIPH_BASE + 0x006C) ) ;
PROVIDE ( DMA_Channel7    = (AHBPERIPH_BASE + 0x0080) ) ;
PROVIDE ( RCC             = (AHBPERIPH_BASE + 0x1000) ) ;

/* System Control Space memory map */
SCS_BASE              = 0xE000E000;

PROVIDE ( SysTick         = (SCS_BASE + 0x0010) ) ;
PROVIDE ( NVIC            = (SCS_BASE + 0x0100) ) ;
PROVIDE ( SCB             = (SCS_BASE + 0x0D00) ) ;


/* Sections Definitions */

SECTIONS
{
    /* for Cortex devices, the beginning of the startup code is stored in the .isr_vector section, which goes to FLASH */
    .isr_vector :
    {
        PROVIDE (pios_isr_vector_table_base = .);
        KEEP(*(.isr_vector))            /* Startup code */
        . = ALIGN(4);
    } > BL_FLASH

    /* for some STRx devices, the beginning of the startup code is stored in the .flashtext section, which goes to FLASH */
    .flashtext :
    {
        . = ALIGN(4);
        *(.flashtext)            /* Startup code */
        . = ALIGN(4);
    } > BL_FLASH
 
    /* the program code is stored in the .text section, which goes to Flash */
    .text :
    {
            . = ALIGN(4);
            
        *(.text)                   /* remaining code */
        *(.text.*)                   /* remaining code */
        *(.rodata)                 /* read-only data (constants) */
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)

            . = ALIGN(4);
         _etext = .;
            /* This is used by the startup in order to initialize the .data secion */
         _sidata = _etext;
    } > BL_FLASH
    

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
         _init_stack_top = _irq_stack_top;
         . = ALIGN(4);
    } >RAM
	

    /* This is the initialized data section
    The program executes knowing that the data is in the RAM
    but the loader puts the initial values in the FLASH (inidata).
    It is one task of the startup to copy the initial values from FLASH to RAM. */
    .data  : AT ( _sidata )
    {
            . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data secion */
        _sdata = . ;
        
        *(.data)
        *(.data.*)
            . = ALIGN(4);
            /* This is used by the startup in order to initialize the .data secion */
         _edata = . ;
    } >RAM
    
    

    /* This is the uninitialized data section */
    .bss :
    {
            . = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss secion */
        _sbss = .;
        
        *(.bss)
        *(COMMON)
        
            . = ALIGN(4);
            /* This is used by the startup in order to initialize the .bss secion */
         _ebss = . ;
    } >RAM
    
    PROVIDE ( end = _ebss );
    PROVIDE ( _end = _ebss );

    /* this is the FLASH Bank1 */
    /* the C or assembly source must explicitly place the code or data there
    using the "section" attribute */
    .b1text :
    {
        *(.b1text)                   /* remaining code */
        *(.b1rodata)                 /* read-only data (constants) */
        *(.b1rodata*)
    } >FLASHB1
    
    /* this is the EXTMEM */
    /* the C or assembly source must explicitly place the code or data there
    using the "section" attribute */
    
    /* EXTMEM Bank0 */
    .eb0text :
    {
        *(.eb0text)                   /* remaining code */
        *(.eb0rodata)                 /* read-only data (constants) */
        *(.eb0rodata*)
    } >EXTMEMB0
    
    /* EXTMEM Bank1 */
    .eb1text :
    {
        *(.eb1text)                   /* remaining code */
        *(.eb1rodata)                 /* read-only data (constants) */
        *(.eb1rodata*)
    } >EXTMEMB1
    
    /* EXTMEM Bank2 */
    .eb2text :
    {
        *(.eb2text)                   /* remaining code */
        *(.eb2rodata)                 /* read-only data (constants) */
        *(.eb2rodata*)
    } >EXTMEMB2
    
    /* EXTMEM Bank0 */
    .eb3text :
    {
        *(.eb3text)                   /* remaining code */
        *(.eb3rodata)                 /* read-only data (constants) */
        *(.eb3rodata*)
    } >EXTMEMB3
    
    __exidx_start = .;
    __exidx_end = .;
    
    .boardinfo :
    {
	. = ALIGN(4);
	KEEP(*(.boardinfo))
	. = ALIGN(4);
    } > BD_INFO

    /* after that it's only debugging information. */
    
    /* remove the debugging information from the standard libraries */
    /DISCARD/ :
    {
     libc.a ( * )
     libm.a ( * )
     libgcc.a ( * )
     }

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
