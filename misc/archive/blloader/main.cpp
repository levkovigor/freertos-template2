extern "C"{
#include <board.h>
#include <AT91SAM9G20.h>
#include <cp15/cp15.h>

#if defined(at91sam9g20_ek)
#include <led_ek.h>
#else
#endif

#include <peripherals/pio/pio.h>
#include <at91/utility/trace.h>

#include <FreeRTOSConfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <hal/Timing/WatchDogTimer.h>

}

#include <fsfw/tasks/TaskFactory.h>

#if defined(at91sam9g20_ek)
void ConfigureLeds(void);
#endif

// quick fix to bypass link error
extern "C" void __sync_synchronize() {}

// This will be the entry to the mission specific code
void initBootloaderLoader();
void initTask(void * args);

int main(void)
{
	//-------------------------------------------------------------------------
	// Configure traces
	//-------------------------------------------------------------------------
	TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);

	TRACE_INFO_WP("\n\r");
	TRACE_INFO_WP("-- SOURCE Bootloader Loader --\n\r");
	TRACE_INFO_WP("-- %s\n\r", BOARD_NAME);
	TRACE_INFO_WP("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

    CP15_Enable_I_Cache();
#if defined(at91sam9g20_ek)
    ConfigureLeds();
    LED_Toggle(0);
#endif

    // Core Task. Custom interrupts should be configured inside a task.
    xTaskCreate(initTask, (const char*)"InitTask", 3072, nullptr, 1, nullptr);
    vTaskStartScheduler();
    // This should never be reached.
    for(;;) {}
}

void initTask (void * args) {
	configASSERT(args == nullptr);
#ifdef ETHERNET
	printf("-- Setting up lwIP Stack and EMAC for UDP/TCP Communication --\n\r");
	emac_lwip_init();
#else
	printf("-- Using Serial Communication --\n\r");
#endif

#ifdef ISIS_OBC_G20
	// Task with the sole purpose of kicking the watchdog to prevent
	// an iOBC restart
	int result = WDT_startWatchdogKickTask(10 / portTICK_RATE_MS, FALSE);
	if(result != 0) {
		TRACE_ERROR("Starting iOBC Watchdog Feed Task failed!\r\n");
	}
#endif

	initBootloaderLoader();
	// Delete self.
    TaskFactory::instance()->deleteTask();
}

/* Configures LEDs \#1 and \#2 (cleared by default). */
#if defined(at91sam9g20_ek)
void ConfigureLeds(void)
{
    LED_Configure(0);
    LED_Configure(1);
}
#endif


