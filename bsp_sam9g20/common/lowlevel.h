#ifndef SAM9G20_UTILITY_LOWLEVEL_H_
#define SAM9G20_UTILITY_LOWLEVEL_H_

#ifdef __cplusplus
extern "C" {
#endif


unsigned int asm_get_cpsr(void);
void asm_set_cpsr(unsigned int val);
void print_processor_state(void);

void asm_disable_irq();
void asm_enable_irq();

void disable_pit_aic();

#ifdef __cplusplus
}
#endif

#endif /* SAM9G20_UTILITY_LOWLEVEL_H_ */
