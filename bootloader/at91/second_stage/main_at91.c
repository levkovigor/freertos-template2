#include "../common/at91_boot_from_nand.h"
#include <bootloaderConfig.h>
#include <core/timer.h>
#include <sam9g20/common/SRAMApi.h>

#include <board.h>
#include <AT91SAM9G20.h>
#include <led_ek.h>
#include <board_memories.h>
#include <peripherals/dbgu/dbgu.h>
#include <peripherals/pio/pio.h>
#include <peripherals/aic/aic.h>
#include <peripherals/pio/pio.h>
#include <cp15/cp15.h>

#include <hal/Timing/RTT.h>
#include <privlib/hcc/include/hcc/api_hcc_mem.h>
#include <sam9g20/common/SDCardApi.h>

#if BOOTLOADER_VERBOSE_LEVEL >= 1
#include <utility/trace.h>
#endif

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <stdbool.h>
#include <string.h>

#define USE_FREERTOS            0
extern void jump_to_sdram_application(uint32_t stack_ptr, uint32_t jump_address);

void init_task(void* args);
void handler_task(void * args);
int perform_bootloader_core_operation();
void initialize_all_peripherals();
void print_bl_info();

#if USE_FREERTOS == 1
static TaskHandle_t handler_task_handle_glob = NULL;
#endif

/**
 * @brief   Bootloader which will be run from SDRAM and is loaded by the first stage bootloader.
 *          It can also be flashed regularly with a debugger probe.
 * @details
 * Currently, there are issues with FreeRTOS, so FreeRTOS support can be disabled optionally.
 * @author  R. Mueller
 */
int at91_main()
{
    //-------------------------------------------------------------------------
    // Configure traces
    //-------------------------------------------------------------------------
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);

    //-------------------------------------------------------------------------
    // Enable I-Cache
    //-------------------------------------------------------------------------
    CP15_Enable_I_Cache();

    //-------------------------------------------------------------------------
    // Configure LEDs and set both of them.
    //-------------------------------------------------------------------------
    LED_Configure(1);
    LED_Configure(0);
    LED_Set(0);
    LED_Set(1);

#if USE_FREERTOS == 0
    /* Activate MS interrupt for timer base */
    setup_timer_interrupt();

    /* Info printout */
#if BOOTLOADER_VERBOSE_LEVEL >= 1
    print_bl_info();
#else
    printf("SOURCEBoot\n\r");
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */


    /* AT91 Bootloader */
    perform_bootloader_core_operation();
#else
    /* Right now, FreeRTOS is problematic, program crashes in portRESTORE_CONTEXT() when
    branching to first task. */
    xTaskCreate(handler_task, "HANDLER_TASK", 1024, NULL, 4, &handler_task_handle_glob);
    xTaskCreate(init_task, "INIT_TASK", 1024, handler_task_handle_glob, 5, NULL);
    TRACE_INFO("Remaining FreeRTOS heap size: %d bytes.\n\r", xPortGetFreeHeapSize());
    // TRACE_INFO("Init Task Address: 0x%08x\n\r", (unsigned int) init_task);

    vTaskStartScheduler();
#endif /* USE_FREERTOS == 0 */

    /* This should never be reached. */
#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_ERROR("FreeRTOS scheduler error!\n\r");
#endif
    for(;;) {};
    return 0;
}

void init_task(void * args) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
    print_bl_info();
#else
    printf("SOURCEBoot\n\r");
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

    initialize_all_peripherals();

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
    else {
        TRACE_ERROR("init_task: handler_task_handle invalid!\n\r");
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

void initialize_all_peripherals() {
}

int perform_bootloader_core_operation() {
    LED_Clear(0);
    LED_Clear(1);

    memset((void*) SDRAM_DESTINATION, 0, PRIMARY_IMAGE_RESERVED_SIZE);

    copy_nandflash_binary_to_sdram(PRIMARY_IMAGE_NAND_OFFSET, PRIMARY_IMAGE_RESERVED_SIZE,
            PRIMARY_IMAGE_SDRAM_OFFSET, false);

    LED_Set(0);

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Jumping to SDRAM application address 0x%08x!\n\r",
            (unsigned int) SDRAM_DESTINATION);
#endif

#if USE_FREERTOS == 1
    vTaskEndScheduler();
#endif
    CP15_Disable_I_Cache();

    jump_to_sdram_application(0x22000000 - 1024, SDRAM_DESTINATION);

    /* Should never be reached */
    return 0;
}

void print_bl_info() {
    TRACE_INFO_WP("\n\r-- SOURCE Bootloader (Second Stage SDRAM) --\n\r");
    TRACE_INFO_WP("-- %s --\n\r", BOARD_NAME_PRINT);
    TRACE_INFO_WP("-- Software version v%d.%d --\n\r", BL_VERSION, BL_SUBVERSION);
    TRACE_INFO_WP("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
}

//VolumeId current_volume = SD_CARD_0;
//int result = open_filesystem();
//if(result != F_NO_ERROR) {
//   /* not good, should not happen.
//   hcc_mem_delete();
//   return -1;
//}
//
//result = select_sd_card(current_volume, true);
//if(result != F_NO_ERROR) {
//   /* not good, should not happen. */
//   close_filesystem(true, true, current_volume);
//   return -1;
//}
//
//result = change_directory(SW_REPOSITORY, true);
//if(result != F_NO_ERROR) {
//#if BOOTLOADER_VERBOSE_LEVEL >= 1
//   TRACE_WARNING("Target SW repository \"%s\" does not exist.\n\r", SW_REPOSITORY);
//#endif
//   /* not good, should not happen. */
//   close_filesystem(true, true, current_volume);
//   return -1;
//}

