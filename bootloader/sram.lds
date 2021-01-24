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
    sram0 (W!RX) : ORIGIN = 0x200000, LENGTH = 0x4000
    sram1 (W!RX) : ORIGIN = 0x300000, LENGTH = 0x4000
    sdram (W!RX) : ORIGIN = 0x20000000, LENGTH = 0x2000000
}

SECTIONS
{  
    .fixed0 :
    {
        . = ALIGN(4);
        _sfixed0 = .;
        KEEP(*(.vectors));
        KEEP(*(.startup)); /* Start up code needed for FreeRTOS */
 		*(.sramfunc)
        *(.text*)
        *(.rodata*)
        *(.data)
        *(.data*)				/* contains all initialized global and static variables */
        *(.glue_7)
        *(.glue_7t)
        *(.CP15_*)
        KEEP (*(.init))
    	KEEP (*(.fini))
        . = ALIGN(4);
        _efixed0 = .;
        _efixed = .;
    } >sram0
    
    	/* used for unwinding, can be useful for debugging  */  
	.ARM.extab   : AT(_efixedsec) 
    {  
		*(.ARM.extab* .gnu.linkonce.armextab.*) 
	} >sram0
   
    /* used for unwinding, can be useful for debugging  */  
	__exidx_start = .;
    .ARM.exidx (NOLOAD) : 
    {
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} >sram0
    __exidx_end = .;

    .fixed1 :
    {
        . = ALIGN(4);
        _sfixed1 = .;
        . = ALIGN(4);
        _efixed1 = .;
    } >sram1

    .prerelocate : AT (_efixed)
    {
        . = ALIGN(4);
        _sprerelocate = .;
        . = ALIGN(4);
        _eprerelocate = .;
    }

    .postrelocate : AT (_efixed + SIZEOF(.prerelocate))
    {
        . = ALIGN(4);
        _spostrelocate = .;
        . = ALIGN(4);
        _epostrelocate = .;
    } 
    
    .sramdata : AT (_sprerelocate + SIZEOF(.prerelocate) + SIZEOF(.postrelocate))
    {
        . = ALIGN(4);
        _ssramdata = .;
        *(.sramdata)
        . = ALIGN(4);
        _esramdata = .;
    } >sram1
    
    .srambss (NOLOAD) : {
        . = ALIGN(4);
        _ssrambss = .;
        *(.srambss)
        . = ALIGN(4);
        _esrambss = .;
    } >sram1

    .bss (NOLOAD) : {
        _szero = .;
        _sbss = .;
        __bss_start__ = _szero;
        *(.bss)
        *(.bss*)
        *(COMMON)
		*(COMMON.*)
        . = ALIGN(4);
        _ezero = .;
        _ebss = .;
        __bss_end__ = _ezero;
    } >sram1
    
    /* Heap section */
	_sheap_ = .;
	end = .;
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    _sstack = 0x304000;

}


