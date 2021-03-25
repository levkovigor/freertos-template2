#include "lowlevel.h"
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



