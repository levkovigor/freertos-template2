#ifndef SAM9G20_UTILITY_TCTIMERHANDLER_H_
#define SAM9G20_UTILITY_TCTIMERHANDLER_H_

#include <fsfw/returnvalues/HasReturnvaluesIF.h>

extern "C" {
#include <AT91SAM9G20.h>
#include <at91/peripherals/tc/tc.h>
}

#include <cstdint>

enum class TcPeripherals {
	TC0,
	TC1,
	TC2,
	TC3,
	TC4,
	TC5
};

using isr_args_t = void *;
using isr_routine_t = void (*)(isr_args_t args);
using isr_routine_aic_t = void (*) (void);

/**
 * @brief	Handler class for the Timer Counter (TC) peripheral of the AT91.
 * @details
 * This handler can be used to generate interrupts or do other tasks.
 * See p.540 of SAM9G20 datasheet for more information.
 * (http://ww1.microchip.com/downloads/en/DeviceDoc/DS60001516A.pdf).
 *
 * Special note for SOURCE: TC0, TC1 and TC2 are used by the ISIS PWM drivers
 * to provide 6 PWM pins!
 */
class TCTimerHandler {
public:
	static const uint8_t LOWEST_ISR_PRIORITY = AT91C_AIC_PRIOR_LOWEST;
	static const uint8_t HIGHEST_ISR_PRIORITY = AT91C_AIC_PRIOR_HIGHEST;

	/**
	 * @brief	Configure a one-shot interrupt. Forwards the call to
	 * 			#configureGenericInterrupt.
	 * @details
	 * The default configuration does not start the timer, which can be
	 * started with startTc() and stopped with stopTc().
	 * Please note that the one-shot interrupt works by disabling the
	 * used timer source.
	 */
	static void configureOneShotInterrupt(TcPeripherals tcSelect,
			void (*isrRoutine)(isr_args_t args),
			uint8_t interruptPriority = LOWEST_ISR_PRIORITY,
			bool startImmediately = false, isr_args_t isrArgs = nullptr);

	/**
	 * @brief 	Configure a periodic interrupt with the timer peripheral.
	 * 			Forwards the call to #configureGenericInterrupt
	 * @details
	 * The slowest frequency possible should be around 0.5 Hz. It is set
     * automatically when passing 0 as an input frequency.
	 */
	static void configurePeriodicInterrupt(TcPeripherals tcSelect,
			uint32_t frequency, void (*isrRoutine)(isr_args_t isrArgs),
			uint8_t interruptPriority = LOWEST_ISR_PRIORITY,
			bool startImmediately = false, isr_args_t isrArgs = nullptr);

	/**
	 * @brief 	Configure a interrupt on timer counter overflow (16 bit)
	 * @details
	 * TC peripheral is configures for an interrupt on overflow by performing
	 * a RC register comparison on the maximum value 0xFFFF.
	 * @param tcSelect
	 * @param frequency
	 * @param isrRoutine
	 * @param interruptPriority
	 * @param isrArgs
	 */
	static void configureOverflowInterrupt(TcPeripherals tcSelect,
				uint32_t frequency, void(*isrRoutine)(isr_args_t isrArgs),
				uint8_t interruptPriority, isr_args_t isrArgs);

	/**
	 * @brief 	Generic function to generate periodic or one-shot interrupts
	 * 			with the timer peripheral.
	 * @details
	 * The default configuration does not start the timer, which can be
	 * started with startTc() and stopped with stopTc().
	 * The TC peripheral is configured for interrupts by performing a
	 * a RC register comparison.
	 * @param tcSelect Which timer peripheral to use.
	 * @param frequency Desired frequency in Hz.
	 * @param isrRoutine ISR routine which takes a void* to an argument.
	 * @param interruptPriority Interrupt priority.
	 * @param startImmediately Specifies whether to start timer immediately.
	 * @param isrArgs This argument is passed to the callback.
	 */
	static void configureGenericInterrupt(TcPeripherals tcSelect,
			uint32_t frequency, void (*isrRoutine)(isr_args_t isrArgs),
			uint8_t interruptPriority = LOWEST_ISR_PRIORITY,
			bool startImmediately = false, isr_args_t isrArgs = nullptr,
			bool oneShotInterrupt = false, bool overflowInterrupt = false);

	static void startTc(TcPeripherals peripheral);
	static void stopTc(TcPeripherals peripheral);

	static AT91S_TC* getPeripheral(TcPeripherals peripheral);
	static uint32_t getPeripheralId(TcPeripherals peripheral);

private:
	static inline bool oneShotIsr[6] = {};
	static inline isr_args_t  isrArgsBuffer[6] = {};
	static inline isr_routine_t isrRoutinesBuffer[6] = {};

	static void tcGenericIsrTc0();
	static void tcGenericIsrTc1();
	static void tcGenericIsrTc2();
	static void tcGenericIsrTc3();
	static void tcGenericIsrTc4();
	static void tcGenericIsrTc5();
	static ReturnValue_t prepareParameters(TcPeripherals tcPeripheral,
			isr_routine_aic_t& aicIsr, uint8_t& interruptPrio,
			void (*isrRoutine)(isr_args_t args), bool oneShotInterrupt,
			isr_args_t isrArgs);
};





#endif /* SAM9G20_UTILITY_TCTIMERHANDLER_H_ */
