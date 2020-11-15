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
 *      Linker script for running in internal SRAM on the AT91SAM9G20
 *----------------------------------------------------------------------------*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(entry)

MEMORY
{
    sram0 (W!RX) : ORIGIN = 0x00200000, LENGTH = 16K
    sram1 (W!RX) : ORIGIN = 0x00300000, LENGTH = 16K
    sdram (W!RX) : ORIGIN = 0x20000000, LENGTH = 32M
    ebi_cs0 (RX) : ORIGIN = 0x10000000, LENGTH = 1M
}

SECTIONS
{          
    .prerelocate : AT (_efixed)
    {
        . = ALIGN(4);
        _sprerelocate = .;
        KEEP(*(.vectors));
        *board_lowlevel.o(.text .text*)
        *board_memories.o(.text .text*)
        . = ALIGN(4);
        _eprerelocate = .;
    } >sram0
    
    .postrelocate : AT (_efixed + SIZEOF(.prerelocate))
    {
        . = ALIGN(4);
        _spostrelocate = .;
        *(.sramfunc*)
        *(.sramdata*)
        /* *HAL.a:*(.text .text.*) */
        *(.data*)
        /* contains all initialized global and static variables */
        . = ALIGN(4);
        _epostrelocate = .;
    } >sram0
    
    .fixed :
    {
        . = ALIGN(4);
        _sfixed = .;
        KEEP(*(.startup));
        *(.text .text.*)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)	 		
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
    } >ebi_cs0

	/* used for unwinding, can be useful for debugging  */  
	.ARM.extab (NOLOAD) :
    {  
		*(.ARM.extab* .gnu.linkonce.armextab.*) 
	} >ebi_cs0
   
    /* used for unwinding, can be useful for debugging  */  
	__exidx_start = .;
    .ARM.exidx (NOLOAD) : 
    {
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} >ebi_cs0
	
	. = ALIGN(4);
    __exidx_end = .;    
    _efixed = .;
    
    .srambss (NOLOAD) : 
    {
    	. = ALIGN(4);
    	_ssrambss = .;
    	*(.srambss*)
    	. = ALIGN(4);
    	_esrambss = .;
    } >sram1

    .bss (NOLOAD) : 
    {
        /* This is used by the startup in order to initialize the .bss secion */
	    _sbss = .;         /* define a global symbol at bss start */
	    __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        
        *(COMMON)
		*(COMMON.*)
        . = ALIGN(4);
        _ebss = .;         /* define a global symbol at bss end */
	    __bss_end__ = _ebss;
    } >sram1
    
    /* Heap section */
	_sheap_ = .;
	end = .;
	_eheap_ = 0x302000;
    
    /* top of stack */
    _sstack = 0x304000;
}

