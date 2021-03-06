/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/*------------------------------------------------------------------------------
 *      Linker script for running in external SDRAM on the AT91SAM9G20
 *----------------------------------------------------------------------------*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(entry)

MEMORY
{
    sram0 (W!RX) : ORIGIN = 0x200000, LENGTH = 0x4000
    sram1 (W!RX) : ORIGIN = 0x300000, LENGTH = 0x4000
    sdram (W!RX) : ORIGIN = 0x20000000, LENGTH = 0x2000000
}

SECTIONS
{     
    /* Will be copied to SRAM at startup, but loaded in SDRAM */
    .prerelocate : AT (_efixed)
    {
        . = ALIGN(4);
        _sprerelocate = .;
        KEEP(*(.vectors));
        *(.sramfunc*);
        . = ALIGN(4);
        _eprerelocate = .;
    } >sram0
    
    /* Will also be copied to SRAM */
    .postrelocate : AT (_efixed + SIZEOF(.prerelocate))
    {
        . = ALIGN(4);
        _spostrelocate = .;
        . = ALIGN(4);
        _epostrelocate = .;
    } >sram0 
         
    .fixed :
	{
        . = ALIGN(4);			/* tells the linker that these section ought to be word aligned */
        _sfixed = .;
        KEEP(*(.startup));
        *(.text*)				/* Code is placed in .text */
        *(.rodata*)				/* global variables marked as const. are placed in .rodata */
        *(.glue_7)				/* glue arm to thumb code */
        *(.glue_7t)				/* glue thumb to arm code */
        *(.eh_frame)
        *(.vfp11_veneer)
        *(.v4_bx)
        *(.data)
        *(.data*)				/* contains all initialized global and static variables */
        *(.dyn)
        *(.iplt)
        *(.igot)
        *(.igot.plt)
        *(.rel.dyn)
        *(.rel.iplt)
		*(.CP15_*)
		KEEP (*(.init))
		KEEP (*(.fini))
        . = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
		
		. = ALIGN(4);
		/* init data, the constructor addresses are placed in this sections */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);
		
		. = ALIGN(4);
		/* finit data, destructor addresses are placed in this section */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);
		. = ALIGN(4);
	} >sdram

	/* used for unwinding, can be useful for debugging  */  
	.ARM.extab :
    {  
		*(.ARM.extab* .gnu.linkonce.armextab.*) 
	} >sdram
   
    /* used for unwinding, can be useful for debugging  */  
	__exidx_start = .;
    .ARM.exidx : 
    {
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} >sdram
    __exidx_end = .;
    
    . = ALIGN(4);
    _efixed = .;

    .sramdata : AT (_efixed + SIZEOF(.prerelocate) + SIZEOF(.postrelocate))
    {
        . = ALIGN(4);
        _ssramdata = .;
        *(.sramdata*)
        . = ALIGN(4);
        _esramdata = .;
    } >sram1
    
    .srambss (NOLOAD) :
    {
        . = ALIGN(4);
        _ssrambss = .;
        *(.srambss)
        . = ALIGN(4);
        _esrambss = .;
    } >sram1

    /* this section holds uninitialized global and static data  */
    .bss (NOLOAD) : 
    {
        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss secion */
        _sbss = .;         /* define a global symbol at bss start */
        __bss_start__ = _sbss;
        *(COMMON)
        *(COMMON.*)
        *(.bss)
        *(.bss*)
        . = ALIGN(4);
        _ebss = .;         /* define a global symbol at bss end */
        __bss_end__ = _ebss;
    } >sdram

    /* Heap section */
    . = ALIGN(4);
	_sheap_ = .;
	PROVIDE ( end = . );
	PROVIDE ( _end = . );
    _eheap_ = 0x21800000;

	/* top of stack */
    _sstack = 0x22000000 - 1024;
    
    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
        libc_nano.a ( * )
    }
}
