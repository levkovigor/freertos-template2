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
 
_Min_Heap_Size = 0x400;      /* required amount of heap  */
_Min_Stack_Size = 0x800; 	 /* required amount of stack */

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
        *(.data)
        *(.data*)				/* contains all initialized global and static variables */
        *(.CP15_*)
        KEEP (*(.init))
    	KEEP (*(.fini))
        . = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
		
		. = ALIGN(4);
		/* init data, the constructors are placed in this sections */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);
		
		. = ALIGN(4);
		/* finit data, destructors are placed in this section */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);
	} >sdram

	.ARM.extab (NOLOAD) : {
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} >sdram 
    
	__exidx_start = .;
		.ARM.exidx (NOLOAD) : {
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > sdram
    __exidx_end = .;
    _efixed = .;

    .prerelocate : AT (_efixed)
    {
        . = ALIGN(4);
        _sprerelocate = .;
        . = ALIGN(4);
        _eprerelocate = .;
    }

    .postrelocate : AT (_sprerelocate + SIZEOF(.prerelocate))
    {
        . = ALIGN(4);
        _spostrelocate = .;
        KEEP(*(.vectors));
		*(.sramfunc)
        *(.ramfunc)
        . = ALIGN(4);
        _epostrelocate = .;
    } >sram0
    

    /* this section holds uninitialized global and static data  */
 	. = ALIGN(4);
	.bss :
  	{
	    /* This is used by the startup in order to initialize the .bss secion */
	    _sbss = .;         /* define a global symbol at bss start */
	    __bss_start__ = _sbss;
	    *(.bss)
	    *(.bss*)
	    *(COMMON)
	
	    . = ALIGN(4);
	    _ebss = .;         /* define a global symbol at bss end */
	    __bss_end__ = _ebss;
  	} >sdram

    /* User_heap_stack section, used to check that there is enough RAM left */
    ._user_heap_stack :
    {
      . = ALIGN(4);
      PROVIDE ( end = . );
      PROVIDE ( _end = . );
      . = . + _Min_Heap_Size;
      . = . + _Min_Stack_Size;
      . = ALIGN(4);
    } >sdram

	/* top of stack */
    _sstack = 0x22000000 - 1024;
}

