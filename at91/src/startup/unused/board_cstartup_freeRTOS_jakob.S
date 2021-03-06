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

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include "board.h"

//------------------------------------------------------------------------------
//         Definitions
//------------------------------------------------------------------------------

#define IRQ_STACK_SIZE   8*3*4

#define ARM_MODE_ABT     0x17
#define ARM_MODE_FIQ     0x11
#define ARM_MODE_IRQ     0x12
#define ARM_MODE_SVC     0x13

#define I_BIT            0x80
#define F_BIT            0x40

//------------------------------------------------------------------------------
//         Startup routine
//------------------------------------------------------------------------------

            .align      4
            .arm
        
/* Exception vectors
 *******************/
            .section    .vectors, "a", %progbits

resetVector:
        ldr     pc, =resetHandler       /* Reset */
undefVector:
        b       undefVector             /* Undefined instruction */
swiVector:
		ldr     pc, =vPortYieldProcessor /* Software interrupt */

prefetchAbortVector:
        b       prefetchAbortVector     /* Prefetch abort */
dataAbortVector:
        b       dataAbortVector         /* Data abort */
reservedVector:
        b       reservedVector          /* Reserved for future use */
irqVector:
        b       irqHandler              /* Interrupt */
fiqVector:
                                        /* Fast interrupt */
//------------------------------------------------------------------------------
/// Handles a fast interrupt request by branching to the address defined in the
/// AIC.
//------------------------------------------------------------------------------
fiqHandler:
        b       fiqHandler
	
//------------------------------------------------------------------------------
/// Handles incoming interrupt requests by branching to the corresponding
/// handler, as defined in the AIC. Supports interrupt nesting.
//------------------------------------------------------------------------------
irqHandler:

/* port save context
 *******************/

	// Push R0 as we are going to use the register.
	// STMDB SP! is a synonym for PUSH and simply pushs the content in R0 to the stack
	STMDB	SP!, {R0}

	// Set R0 to point to the task stack pointer.
	STMDB	SP, {SP}^
	NOP
	SUB		SP, SP, #4
	LDMIA	SP!, {R0}

	// Push the return address onto the stack.
	STMDB	R0!, {LR}

	// Now we have saved LR we can use it instead of R0.
	MOV		LR, R0

	// Pop R0 so we can save it onto the system mode stack.
	LDMIA	SP!, {R0}

	// Push all the system mode registers onto the task stack.
	STMDB	LR, {R0-LR}^
	NOP
	SUB		LR, LR, #60

	// Push the SPSR onto the task stack.
	MRS		R0, SPSR
	STMDB	LR!, {R0}

	LDR		R0, =ulCriticalNesting
	LDR		R0, [R0]
	STMDB	LR!, {R0}

	// Store the new top of stack for the task.
	LDR		R1, =pxCurrentTCB
	LDR		R0, [R1]
	STR		LR, [R0]

/* load ISR
 **********/

    /* Write in the IVR to support Protect Mode */
    LDR     lr, =AT91C_BASE_AIC
    LDR     r0, [r14, #AIC_IVR]
    STR     lr, [r14, #AIC_IVR]

    /* Branch to C portion of the interrupt handler */
    MOV     lr, pc
    BX      r0

    /* Acknowledge interrupt */
    LDR     lr, =AT91C_BASE_AIC
    STR     lr, [r14, #AIC_EOICR]

/* port restore context
 **********************/

	// Set the LR to the task stack.
	LDR		R1, =pxCurrentTCB
	LDR		R0, [R1]
	LDR		LR, [R0]

	// The critical nesting depth is the first item on the stack.
	// Load it into the ulCriticalNesting variable.
	LDR		R0, =ulCriticalNesting
	LDMFD	LR!, {R1}
	STR		R1, [R0]

	// Get the SPSR from the stack.
	LDMFD	LR!, {R0}
	MSR		SPSR_cxsf, R0

	// Restore all system mode registers for the task.
	LDMFD	LR, {R0-R14}^
	NOP

	// Restore the return address.
	LDR		LR, [LR, #+60]

	// And return - correcting the offset in the LR to obtain the
	// correct address.
	SUBS	PC, LR, #4

//------------------------------------------------------------------------------
/// Initializes the chip and branches to the main() function.
//------------------------------------------------------------------------------
            .section    .startup
            .global     entry

entry:
resetHandler:

/* Useless instruction for referencing the .vectors section */
        ldr     r0, =resetVector

/* Set pc to actual code location (i.e. not in remap zone) */
	    ldr     pc, =1f

/* Initialize the prerelocate segment */
1:
        ldr     r0, =_efixed
        ldr     r1, =_sprerelocate
        ldr     r2, =_eprerelocate
1:
        cmp     r1, r2
        ldrcc   r3, [r0], #4
        strcc   r3, [r1], #4
        bcc     1b

/* Perform low-level initialization of the chip using LowLevelInit() */
        ldr     sp, =_sstack
        stmfd   sp!, {r0}
	    ldr     r0, =LowLevelInit
        blx     r0

/* Initialize the postrelocate segment */

        ldmfd   sp!, {r0}
        ldr     r1, =_spostrelocate
        ldr     r2, =_epostrelocate
1:
        cmp     r1, r2
        ldrcc   r3, [r0], #4
        strcc   r3, [r1], #4
        bcc     1b

#ifdef sdram
/* Initialize the sram data segment */
/* Only execute this when the code is running from SDRAM */
/* When the code runs from NORFLASH this part of the postrelocate section which is handled above */
        ldr     r1, =_ssramdata
        ldr     r2, =_esramdata
1:
        cmp     r1, r2
        ldrcc   r3, [r0], #4
        strcc   r3, [r1], #4
        bcc     1b
#endif

/* Clear the zero segment */
	    ldr     r0, =__bss_start__
        ldr     r1, =__bss_end__
        mov     r2, #0
1:
        cmp     r0, r1
        strcc   r2, [r0], #4
        bcc     1b

#ifdef sdram
/* Only execute this when the code is running from SDRAM */
/* When the code runs from NORFLASH this is the same as normal bss which is handled above */
/* Clear the SRAM zero segment */
        ldr     r0, =_ssrambss
        ldr     r1, =_esrambss
        mov     r2, #0
1:
        cmp     r0, r1
        strcc   r2, [r0], #4
        bcc     1b
#endif

/* Setup stacks
 **************/
/* IRQ mode */
        msr     CPSR_c, #ARM_MODE_IRQ | I_BIT | F_BIT
        ldr     sp, =_sstack
        sub     r4, sp, #IRQ_STACK_SIZE

/* Supervisor mode (interrupts enabled) */
        msr     CPSR_c, #ARM_MODE_SVC | F_BIT
        mov     sp, r4

/* Call static constructors
 * this call deactivates the interrupts agein
 * interrutps are enabled agein when the freertos scheduler starts
 * libc_init_array runs through the list of static constructor addresses
 * placed in the .init_array and calls all static constructors (in case of the FSFW for example the DataPool::DataPool)
 */
		ldr r0,=__libc_init_array
		mov lr,pc /* set the return address */
		bx r0 /* the target code can be ARM or THUMB */

/* Branch to main()
 ******************/
        ldr     r0, =main
        blx     r0

/* Loop indefinitely when program is finished */
1:
        b       1b

