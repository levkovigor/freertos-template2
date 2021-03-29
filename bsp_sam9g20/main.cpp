extern "C"{
#include <board.h>
#include <AT91SAM9G20.h>
#include <at91/peripherals/cp15/cp15.h>
#include <at91/peripherals/pio/pio.h>
#include <at91/utility/trace.h>

#if defined(AT91SAM9G20_EK)
#include <led_ek.h>
#else
#include <hal/Drivers/LED.h>
<<<<<<< HEAD:sam9g20/main.cpp
#include <sam9g20/common/watchdog.h>
=======
#include <bsp_sam9g20/common/watchdog.h>
>>>>>>> mueller/master:bsp_sam9g20/main.cpp
#endif /* !defined(AT91SAM9G20_EK) */

#include <hal/Timing/WatchDogTimer.h>
#include <hal/Storage/FRAM.h>

#ifdef ETHERNET
#include <emac.h>
#include <macb.h>
#include <emacif.h>
#include <lwip_init.h>
#include <lwip/debug.h>
#include <lwip/arch.h>
extern struct netif *netif;
#endif
}

<<<<<<< HEAD:sam9g20/main.cpp
#include <sam9g20/common/SRAMApi.h>
=======
#include <bsp_sam9g20/common/SRAMApi.h>
>>>>>>> mueller/master:bsp_sam9g20/main.cpp
#include <fsfw/tasks/TaskFactory.h>

#include <FreeRTOSConfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

<<<<<<< HEAD:sam9g20/main.cpp
// quick fix to bypass link error
=======
/* quick fix to bypass link error */
>>>>>>> mueller/master:bsp_sam9g20/main.cpp
extern "C" void __sync_synchronize() {}

#if defined(AT91SAM9G20_EK)
void ConfigureLeds(void);
void configureEk(void);
#endif
unsigned int asm_get_cpsr(void);
void asm_set_cpsr(unsigned int val);
void printProcessorState(void);

/* This will be the entry to the mission specific code */
void initMission();
void initTask(void * args);

#ifdef ISIS_OBC_G20
static constexpr uint32_t WATCHDOG_KICK_INTERVAL_MS = 15;
#endif


int main(void)
{
    BaseType_t retval = pdFALSE;
<<<<<<< HEAD:sam9g20/main.cpp

    // DBGU output configuration
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);

    // Enable Co-Processor instruction cache.
    CP15_Enable_I_Cache();
=======
>>>>>>> mueller/master:bsp_sam9g20/main.cpp

    /* DBGU output configuration */
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);

    /* Enable Co-Processor instruction cache. */
    CP15_Enable_I_Cache();

#ifdef ISIS_OBC_G20
    /* Task with the sole purpose of kicking the watchdog to prevent
    an iOBC restart. This should be done as soon as possible and before
    anything is printed. */
    retval = startCustomIsisWatchdogTask(WATCHDOG_KICK_INTERVAL_MS, true);
    if(retval != pdTRUE) {
        TRACE_ERROR("Starting iOBC Watchdog Feed Task failed!\r\n");
    }
#endif

#if defined(AT91SAM9G20_EK)
    ConfigureLeds();
    configureEk();
#endif

#ifdef ISIS_OBC_G20
    /* Core Task. Custom interrupts should be configured inside a task.
    Less priority than the watchdog task, but still very high to it can
    initiate the software as fast as possible */
    retval = xTaskCreate(initTask, "INIT_TASK", 3072, nullptr, configMAX_PRIORITIES - 2, nullptr);
#else
    retval = xTaskCreate(initTask, "INIT_TASK", 3072, nullptr, 9, nullptr);
#endif
    if(retval != pdTRUE) {
        TRACE_ERROR("Creating Initialization Task failed!\n\r");
    }

<<<<<<< HEAD:sam9g20/main.cpp
    // printProcessorState();
=======
>>>>>>> mueller/master:bsp_sam9g20/main.cpp
    vTaskStartScheduler();
    /* This should never be reached. */
    for(;;) {}
}

void initTask (void * args) {
    //TRACE_INFO("initTask reached.\n\r");
    configASSERT(args == nullptr);

    initMission();

<<<<<<< HEAD:sam9g20/main.cpp
    // Delete self.
=======
    /* Delete self. */
>>>>>>> mueller/master:bsp_sam9g20/main.cpp
    TaskFactory::instance()->deleteTask();
}

/* Configures LEDs \#1 and \#2 (cleared by default). */
#if defined(AT91SAM9G20_EK)
void ConfigureLeds(void)
{
    LED_Configure(0);
    LED_Configure(1);
}

/* Configure PIO for evaluation board. */
void configureEk(void) {

}
#endif

<<<<<<< HEAD:sam9g20/main.cpp
unsigned int asm_get_cpsr(void) {
  unsigned long retval;
  asm volatile (" mrs  %0, cpsr" : "=r" (retval) : /* no inputs */  );
  return retval;
}

void asm_set_cpsr(unsigned int val) {
    asm volatile (" msr cpsr, %0" : /* no outputs */ : "r" (val) );
}

void printProcessorState(void) {
    int cpsr = asm_get_cpsr();
    TRACE_INFO("CPSR: 0x%08x\n\r", cpsr);
    register int stack_ptr asm("sp");
    TRACE_INFO("Stack pointer: 0x%08x\n\r", stack_ptr);
}
=======
>>>>>>> mueller/master:bsp_sam9g20/main.cpp

