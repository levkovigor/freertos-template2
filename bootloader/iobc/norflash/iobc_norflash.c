#include "main.h"
#include "config/bootloaderConfig.h"

#include <iobc/bootIOBC.h>
#include <sam9g20/memory/SDCardApi.h>
#include <sam9g20/common/FRAMApi.h>

#include <board.h>
#include <AT91SAM9G20.h>
#include <board_memories.h>
#include <peripherals/dbgu/dbgu.h>
#include <peripherals/pio/pio.h>
#include <peripherals/aic/aic.h>
#include <peripherals/pio/pio.h>
#include <cp15/cp15.h>
#if DEBUG_IO_LIB == 1
#include <utility/trace.h>
#endif

#include <hal/Drivers/LED.h>
#include <hal/Timing/RTT.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <FreeRTOSConfig.h>
#include <hal/Timing/WatchDogTimer.h>
#include <hal/Storage/FRAM.h>
#include <hal/Storage/NORflash.h>

#include <stdbool.h>
#include <string.h>

void init_task(void* args);
void handler_task(void * args);
void initialize_all_iobc_peripherals();

void idle_loop();

static TaskHandle_t handler_task_handle_glob = NULL;

static const uint32_t WATCHDOG_KICK_INTERVAL_MS = 15;

int iobc_norflash() {
    //-------------------------------------------------------------------------
    // Configure traces
    //-------------------------------------------------------------------------
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);

    //-------------------------------------------------------------------------
    // Initiate watchdog for iOBC
    //-------------------------------------------------------------------------
    int retval = WDT_startWatchdogKickTask(
            WATCHDOG_KICK_INTERVAL_MS / portTICK_RATE_MS, 0);
    if(retval != 0) {
#if DEBUG_IO_LIB == 1
        TRACE_ERROR("Starting iOBC Watchdog Feed Task failed!\r\n");
#endif
    }

    //-------------------------------------------------------------------------
    // Enable I-Cache
    //-------------------------------------------------------------------------
    CP15_Enable_I_Cache();

    LED_start();
    LED_glow(led_2);
    LED_glow(led_3);
    LED_glow(led_4);

#ifndef ISIS_OBC_G20
    feed_watchdog_if_necessary();
#endif

    //-------------------------------------------------------------------------
    // iOBC Bootloader
    //-------------------------------------------------------------------------
    // otherwise, try to copy SDCard binary to SDRAM
    // Core Task. Custom interrupts should be configured inside a task.
    xTaskCreate(handler_task, "HANDLER_TASK", 512, NULL, 2,
            &handler_task_handle_glob);
    xTaskCreate(init_task, "INIT_TASK", 512, handler_task_handle_glob,
            3, NULL);
    vTaskStartScheduler();
#if DEBUG_IO_LIB == 1
    TRACE_ERROR("FreeRTOS scheduler error!\n\r");
#endif
    for(;;) {};
    return 0;
}



void init_task(void * args) {
#if DEBUG_IO_LIB == 1
    TRACE_INFO("\n\rStarting FreeRTOS task scheduler.\n\r");
    TRACE_INFO_WP("-- SOURCE Bootloader --\n\r");
    TRACE_INFO_WP("-- %s --\n\r", BOARD_NAME);
    TRACE_INFO_WP("-- Software version v%d.%d --\n\r", SW_VERSION, SW_SUBVERSION);
    TRACE_INFO_WP("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
    TRACE_INFO("Running initialization task..\n\r");
#else
    TRACE_INFO("\n\rSOURCE Bootloader\n\r");
#endif
    initialize_all_iobc_peripherals();
    // perform initialization which needs to be inside a task.

    // start handler task
    TaskHandle_t handler_task_handle = (TaskHandle_t) args;
    if(handler_task_handle != NULL) {
        while(eTaskGetState(handler_task_handle) != eSuspended) {
            vTaskDelay(1);
        }
        vTaskResume(handler_task_handle);
    }

    // Initialization task not needed anymore, deletes itself.
    vTaskDelete(NULL);
}


void handler_task(void * args) {
#if DEBUG_IO_LIB == 1
    TRACE_INFO("Running handler task..\n\r");
#endif
    // Wait for initialization to finish
    vTaskSuspend(NULL);

    perform_bootloader_core_operation();

    // will not be reached when bootloader is finished.
    //idle_loop();
}

void initialize_all_iobc_peripherals() {
    RTT_start();
    int result = FRAM_start();
    if(result != 0) {
        // This should not happen!
        TRACE_ERROR("initialize_iobc_peripherals: Could not start FRAM!\n\r");
    }
}


int copy_sdcard_binary_to_sdram(BootSelect boot_select) {
	int result = open_filesystem(SD_CARD_0);
	VolumeId current_filesystem = SD_CARD_0;
	if(result != 0) {
		// should not happen..
		result = open_filesystem(SD_CARD_1);
		if(result != 0) {
			// not good at all. boot from NOR-Flash instead
			return -1;
		}
		current_filesystem = SD_CARD_1;
		if ((boot_select == BOOT_SD_CARD_0) ||
				(boot_select == BOOT_SD_CARD_0_SLOT2)) {
			// just take the first binary for now
			boot_select = BOOT_SD_CARD_1;
		}
	}

	switch(boot_select) {
	case(BOOT_SD_CARD_0): {
		// get repostiory
		char bin_folder_name[16];
		// hardcoded
		//int result = read_sdc_bin_folder_name(bin_folder_name);
		if(result != 0) {
			// not good
		}
		result = change_directory(bin_folder_name, true);
		if(result != F_NO_ERROR) {
			// not good
		}
		char binary_name[16];
		char hamming_name[16];
		// hardcoded
		//result = read_sdc1sl1_bin_names(binary_name, hamming_name);
		if(result != 0) {

		}
		// read in buckets.. multiple of 256. maybe 10 * 256 bytes?
		F_FILE* file = f_open(binary_name, "r");
		if (f_getlasterror() != F_NO_ERROR) {
			// opening file failed!
			return -1;
		}
		// get total file size.
		long filesize = f_filelength(binary_name);
		size_t current_idx = 0;
		size_t read_bucket_size = 10 * 256;
		while(true) {
			// we are close to the end of the file and don't have to read
			// the full bucket size
			if(filesize - current_idx < read_bucket_size) {
				read_bucket_size = filesize - current_idx;
			}
			// copy to SDRAM directly
			size_t bytes_read = f_read(
					(void *) (SDRAM_DESTINATION + current_idx),
					sizeof(uint8_t),
					read_bucket_size,
					file);
			if(bytes_read == -1) {
				// this should definitely not happen
				// we need to ensure we will not be locked in a permanent loop.
			}
			else if(bytes_read < 10 * 256) {
				// should not happen!
			}

			// now we could perform the hamming check on the RAM code directly
			// If the last bucket is smaller than 256, we pad with 0
			// and assume the hamming code calculation was performed with
			// padding too.
			current_idx += bytes_read;
			if(current_idx >= filesize) {
				break;
			}

		}


		break;
	}
	case(BOOT_SD_CARD_0_SLOT2): {
		break;
	}
	case(BOOT_SD_CARD_1): {
		break;
	}
	case(BOOT_SD_CARD_1_SLOT2): {
		break;
	}
	case(BOOT_NOR_FLASH): {
		return -1;
	}
	}
	close_filesystem(current_filesystem);
	return 0;
}
