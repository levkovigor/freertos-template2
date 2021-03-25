#ifndef SAM9G20_UTILITY_LOWLEVEL_H_
#define SAM9G20_UTILITY_LOWLEVEL_H_

unsigned int asm_get_cpsr(void);
void asm_set_cpsr(unsigned int val);
void print_processor_state(void);

void asm_disable_irq();
void asm_enable_irq();

#endif /* SAM9G20_UTILITY_LOWLEVEL_H_ */
