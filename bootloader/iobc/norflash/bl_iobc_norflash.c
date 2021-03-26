#include "bl_iobc_norflash.h"
#include "../common/boot_iobc.h"
#include <bootloaderConfig.h>
#include <commonIOBCConfig.h>

#include <bootloader/utility/CRC.h>
#include <bsp_sam9g20/common/SRAMApi.h>

#include <board.h>
#include <AT91SAM9G20.h>
#include <board_memories.h>
#include <at91/peripherals/dbgu/dbgu.h>
#include <at91/peripherals/pio/pio.h>
#include <at91/peripherals/aic/aic.h>
#include <at91/peripherals/pio/pio.h>
#include <at91/peripherals/cp15/cp15.h>

#if BOOTLOADER_VERBOSE_LEVEL >= 1
#include <utility/trace.h>
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

#if USE_FREERTOS == 1
#include <bsp_sam9g20/common/FRAMApi.h>
#include <bsp_sam9g20/common/watchdog.h>

#include <FreeRTOSConfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <hal/Storage/FRAM.h>
#endif

#include <hal/Drivers/LED.h>
#include <hal/Timing/RTT.h>
#include <hal/Timing/WatchDogTimer.h>
#include <hal/Storage/NORflash.h>

#include <stdbool.h>
#include <string.h>

#if USE_FREERTOS == 1
void init_task(void* args);
void handler_task(void * args);
static TaskHandle_t handler_task_handle_glob = NULL;
static const uint32_t WATCHDOG_KICK_INTERVAL_MS = 15;

#else

void simple_bootloader();

#endif

void perform_bootloader_check();
void initialize_all_iobc_peripherals();
#if BOOTLOADER_VERBOSE_LEVEL >= 1
void print_bl_info();
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

bool fram_faulty = false;


int boot_iobc_from_norflash() {
    //-------------------------------------------------------------------------
    // Configure traces
    //-------------------------------------------------------------------------
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);

#if USE_FREERTOS == 1
    //-------------------------------------------------------------------------
    // Initiate watchdog for iOBC
    //-------------------------------------------------------------------------
    BaseType_t retval = startCustomIsisWatchdogTask(WATCHDOG_KICK_INTERVAL_MS,
            true);
    if(retval != pdTRUE) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_ERROR("Starting iOBC Watchdog Feed Task failed!\r\n");
#endif
    }
#endif /* USE_FREERTOS == 1 */

    //-------------------------------------------------------------------------
    // Enable I-Cache
    //-------------------------------------------------------------------------
    CP15_Enable_I_Cache();

    // Glow all LEDs
    LED_start();
    LED_glow(led_2);
    LED_glow(led_3);
    LED_glow(led_4);

#if USE_FREERTOS == 1
    /* iOBC Bootloader */
    xTaskCreate(handler_task, "HANDLER_TASK", 2048, NULL, 4, &handler_task_handle_glob);
    xTaskCreate(init_task, "INIT_TASK", 1024, handler_task_handle_glob, 5, NULL);
    vTaskStartScheduler();
#else
    simple_bootloader();
#endif /* !USE_FREERTOS == 1 */

    /* This should never be reached. */
#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_ERROR("FreeRTOS scheduler error!\n\r");
#endif
    for(;;) {};
    return 0;
}

#if USE_FREERTOS == 1

void init_task(void * args) {
    /* This check is only possible if CRC and bootloader size were written
    at special memory locations. SAM-BA can't do this. */
#if SAM_BA_BOOT == 0
    /* We do this check inside a task so that the watchdog task can take care of
    feeding the watchdog. */
    perform_bootloader_check();
#endif /* SAM_BA_BOOT */

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    print_bl_info();
    TRACE_INFO("Remaining FreeRTOS heap size: %d bytes.\n\r", xPortGetFreeHeapSize());
#else
    printf("SOURCEBoot\n\r");
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

    initialize_all_iobc_peripherals();

    /* Start handler task */
    TaskHandle_t handler_task_handle = (TaskHandle_t) args;
    if(handler_task_handle != NULL) {
        /* Wait till the handler task is suspended */
        while(eTaskGetState(handler_task_handle) != eSuspended) {
            vTaskDelay(1);
        }
        /* Initialization is finished and the handler task can start */
        vTaskResume(handler_task_handle);
    }

    /* Initialization task not needed anymore, deletes itself. */
    vTaskDelete(NULL);
}


void handler_task(void * args) {
    /* Wait for initialization to finish */
    vTaskSuspend(NULL);

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Running handler task..\n\r");
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

    perform_bootloader_core_operation();
}

#else /* USE_FREERTOS == 1 */

void simple_bootloader() {
    TRACE_INFO_WP("-- SOURCE Bootloader v%d.%d --\n\r", BL_VERSION, BL_SUBVERSION);
    /* We don't need these */
    // initialize_all_iobc_peripherals();
    int result = copy_norflash_binary_to_sdram(PRIMARY_IMAGE_RESERVED_SIZE, false);
    if(result != 0) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("Copy operation error\n\r");
#endif
    }
#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Jumping to SDRAM application\n\r");
#endif
    jump_to_sdram_application(0x22000000 - 1024, SDRAM_DESTINATION);
}

#endif /* !USE_FREERTOS == 1 */

void print_bl_info() {
    TRACE_INFO_WP("\n\rStarting FreeRTOS task scheduler.\n\r");
    TRACE_INFO_WP("-- SOURCE Bootloader --\n\r");
    TRACE_INFO_WP("-- %s --\n\r", BOARD_NAME_PRINT);
    TRACE_INFO_WP("-- Software version v%d.%d --\n\r", BL_VERSION,
            BL_SUBVERSION);
    TRACE_INFO_WP("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
    TRACE_INFO("Running initialization task..\n\r");
}

void perform_bootloader_check() {
    /* Check CRC of bootloader:

	If CRC16 is blank (0x00, 0xff), continue and emit warning (it is recommended
	to write the CRC field when writing the bootloader. If SAM-BA is used
	this can also be perform in software)

	If not, check it by calculating CRC16 with the given bootloader size.
	If it is invalid, copy binary and jump there
	immediately to reduce number of  instructions. We also set a special
	variable at the end of SRAM0 to notify the primary software that the
	bootloader is faulty. */

    uint16_t written_crc16 = 0;
    size_t bootloader_size = 0;
    /* Bootloader size and CRC16 are written at the end of the reserved bootloader space. */
    memcpy(&bootloader_size, (const void *) NORFLASH_BL_SIZE_START_READ, 4);

#if BOOTLOADER_VERBOSE_LEVEL >= 2
    TRACE_INFO("Written bootloader size: %d bytes.\n\r", bootloader_size);
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

    memcpy(&written_crc16, (const void*) NORFLASH_BL_CRC16_START_READ, sizeof(written_crc16));

#if BOOTLOADER_VERBOSE_LEVEL >= 2
    TRACE_INFO("Written CRC16: 0x%4x.\n\r", written_crc16);
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

    if(written_crc16 != 0x00 || written_crc16 != 0xff) {
        uint16_t calculated_crc = crc16ccitt_default_start_crc(
                (const void *) BOOTLOADER_BASE_ADDRESS_READ, bootloader_size);
        if(written_crc16 != calculated_crc) {
            memcpy((void*)SDRAM_DESTINATION, (const void*) BINARY_BASE_ADDRESS_READ,
                    PRIMARY_IMAGE_RESERVED_SIZE);
#if BOOTLOADER_VERBOSE_LEVEL >= 1
            TRACE_WARNING("Bootloader CRC check failed. Copying and jumping to "
                    "NOR-Flash image..\n\r");
#endif
            set_sram0_status_field(SRAM_BOOTLOADER_INVALID);
            vTaskEndScheduler();
            jump_to_sdram_application(0x22000000 - 1024, SDRAM_DESTINATION);
        }
    }
    else {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("CRC field is blank!\n\r");
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */
    }
}

void initialize_all_iobc_peripherals() {
    RTT_start();

#if USE_FREERTOS == 1
    int result = FRAM_start();
    if(result != 0) {
        // This should not happen!
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_ERROR("initialize_iobc_peripherals: Could not start FRAM, code %d!\n\r", result);
        set_sram0_status_field(SRAM_FRAM_ISSUES);
        fram_faulty = true;
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */
    }
#endif /* USE_FREERTOS == 1 */
}

