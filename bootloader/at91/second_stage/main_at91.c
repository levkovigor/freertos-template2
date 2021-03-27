#include "../common/at91_boot_from_nand.h"
#include "at91_boot_from_sd.h"
#include <bootloaderConfig.h>
#include <bootloader/core/timer.h>

#include <bsp_sam9g20/common/SRAMApi.h>
#include <bsp_sam9g20/common/VirtualFRAMApi.h>
#include <bsp_sam9g20/common/lowlevel.h>

#include <at91/boards/at91sam9g20-ek/board.h>
#include <at91/boards/at91sam9g20-ek/at91sam9g20/AT91SAM9G20.h>
#include <at91/boards/at91sam9g20-ek/led_ek.h>
#include <at91/boards/at91sam9g20-ek/board_memories.h>
#include <at91/peripherals/pit/pit.h>
#include <at91/peripherals/dbgu/dbgu.h>
#include <at91/peripherals/pio/pio.h>
#include <at91/peripherals/aic/aic.h>
#include <at91/peripherals/pio/pio.h>
#include <at91/peripherals/cp15/cp15.h>
#include <at91/utility/trace.h>

#include <hal/Timing/RTT.h>

#if USE_FREERTOS == 1
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#endif

#include <stdbool.h>
#include <string.h>

extern void jump_to_sdram_application(uint32_t stack_ptr, uint32_t jump_address);

#if USE_FREERTOS == 1
void init_task(void* args);
void handler_task(void * args);
#endif

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
    /* Activate MS interrupt for timer base. Again, consider disabling
    because of issues with FreeRTOS */
    //setup_timer_interrupt();

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
    branching to first task sometimes. It was not possible to solve this problem. */
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

#if USE_FREERTOS == 1
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
#endif

void initialize_all_peripherals() {
}

int perform_bootloader_core_operation() {
    LED_Clear(0);
    LED_Clear(1);

    int result = 0;
    //result = copy_sdc_image_to_sdram();
    result = copy_nandflash_image_to_sdram(PRIMARY_IMAGE_NAND_OFFSET, PRIMARY_IMAGE_RESERVED_SIZE,
            PRIMARY_IMAGE_SDRAM_OFFSET, false);

    if(result != 0) {};

    LED_Set(0);

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Jumping to SDRAM application address 0x%08x!\n\r",
            (unsigned int) SDRAM_DESTINATION);
#endif

#if USE_FREERTOS == 1
    vTaskEndScheduler();
#endif

    CP15_Disable_I_Cache();
    //disable_pit_aic();

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


