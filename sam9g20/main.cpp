
extern "C"{
#include <board.h>
#include <AT91SAM9G20.h>
#include <at91/peripherals/cp15/cp15.h>

#if defined(AT91SAM9G20_EK)
#include <led_ek.h>
#else
#endif

#include <at91/peripherals/pio/pio.h>
#include <at91/utility/trace.h>

#include <FreeRTOSConfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <hal/Timing/WatchDogTimer.h>
#include <hal/Storage/FRAM.h>
#include <sam9g20/common/FRAMApi.h>

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

#include <fsfw/tasks/TaskFactory.h>

// quick fix to bypass link error
extern "C" void __sync_synchronize() {}

#if defined(AT91SAM9G20_EK)
void ConfigureLeds(void);
void configureEk(void);
#endif

// This will be the entry to the mission specific code
void initMission();
void initTask(void * args);

#ifdef ISIS_OBC_G20
static const uint8_t WATCHDOG_KICK_INTERVAL_MS = 15;
#endif

int main(void)
{
    // DBGU output configuration
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);

#ifdef ISIS_OBC_G20
	// Task with the sole purpose of kicking the watchdog to prevent
	// an iOBC restart. This should be done as soon as possible and before
    // anything is printed.
	int retval = WDT_startWatchdogKickTask(
			WATCHDOG_KICK_INTERVAL_MS / portTICK_RATE_MS, TRUE);
	if(retval != 0) {
		TRACE_ERROR("Starting iOBC Watchdog Feed Task failed!\r\n");
	}
#endif

    // Enable Co-Processor instruction cache.
    CP15_Enable_I_Cache();

#if defined(AT91SAM9G20_EK)
    ConfigureLeds();
    configureEk();
    LED_Toggle(0);
#endif
    // Kick might be necessary if initTask suppresses watchdog task for too long
#ifdef ISIS_OBC_G20
    // WDT_forceKick();

    // Core Task. Custom interrupts should be configured inside a task.
    xTaskCreate(initTask, "INIT_TASK", 3072, nullptr, 6, nullptr);
#else
    xTaskCreate(initTask, "INIT_TASK", 3072, nullptr, 9, nullptr);
#endif
    vTaskStartScheduler();
    // This should never be reached.
    for(;;) {}
}

void initTask (void * args) {
	configASSERT(args == nullptr);

	initMission();
	// Delete self.
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
