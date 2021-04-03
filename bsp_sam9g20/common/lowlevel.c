#include "lowlevel.h"
#include <at91/peripherals/pit/pit.h>
#include <at91/peripherals/aic/aic.h>
#include <at91/utility/trace.h>

#define I_BIT 0x80

unsigned int asm_get_cpsr(void) {
  unsigned long retval;
  asm volatile (" mrs  %0, cpsr" : "=r" (retval) : /* no inputs */  );
  return retval;
}

void asm_set_cpsr(unsigned int val) {
    asm volatile (" msr cpsr, %0" : /* no outputs */ : "r" (val) );
}

void asm_disable_irq() {
    unsigned long retval = asm_get_cpsr();
    retval &= ~I_BIT;
    asm_set_cpsr(retval);
}

void asm_enable_irq() {
    unsigned long retval = asm_get_cpsr();
    retval |= I_BIT;
    asm_set_cpsr(retval);
}

void print_processor_state(void) {
    int cpsr = asm_get_cpsr();
    TRACE_INFO("CPSR: 0x%08x\n\r", cpsr);
    register int stack_ptr asm("sp");
    TRACE_INFO("Stack pointer: 0x%08x\n\r", stack_ptr);
}

void disable_pit_aic() {
    // Unstack nested interrupts
    for (int i = 0; i < 8 ; i++) {
        AT91C_BASE_AIC->AIC_EOICR = 0;
    }
    // Clear AIC and PIT interrupts and disable them.
    AT91C_BASE_AIC->AIC_ICCR = 1 << AT91C_ID_SYS;
    AIC_DisableIT( AT91C_ID_SYS );
    PIT_GetPIVR();
    PIT_DisableIT();
    PIT_Disable();
    AT91C_BASE_PITC->PITC_PIMR = 0;
}




