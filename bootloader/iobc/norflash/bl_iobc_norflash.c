#include "bl_iobc_norflash.h"
#include "../common/boot_iobc.h"
#include <bootloaderConfig.h>
#include <commonIOBCConfig.h>

#include <bootloader/utility/CRC.h>
#include <bsp_sam9g20/common/lowlevel.h>
#include <bootloader/core/timer.h>
#include <bsp_sam9g20/common/SRAMApi.h>

#include <board.h>
#include <AT91SAM9G20.h>

#include <at91/peripherals/dbgu/dbgu.h>
#include <at91/peripherals/pio/pio.h>
#include <at91/peripherals/aic/aic.h>
#include <at91/peripherals/pio/pio.h>
#include <at91/peripherals/cp15/cp15.h>

#if BOOTLOADER_VERBOSE_LEVEL >= 1
#include <utility/trace.h>
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

#if USE_FREERTOS == 1
#include <bsp_sam9g20/common/fram/FRAMApi.h>
#include <bsp_sam9g20/common/watchdog.h>

#include <FreeRTOSConfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <hal/Storage/FRAM.h>
#include <hal/Timing/WatchDogTimer.h>
#else

#include <hal/Storage/FRAM.h>
#include <bsp_sam9g20/common/fram/FRAMNoOs.h>
#include <bsp_sam9g20/common/fram/FRAMApiNoOs.h>
#include <hal/Timing/WatchDogTimerNoOS.h>

#endif

#include <hal/Drivers/LED.h>
#include <hal/Timing/RTT.h>
#include <hal/Storage/NORflash.h>

#include <stdbool.h>
#include <string.h>

#if USE_FREERTOS == 1
void init_task(void* args);
void handler_task(void * args);
static TaskHandle_t handler_task_handle_glob = NULL;
static const uint32_t WATCHDOG_KICK_INTERVAL_MS = 15;

#else

BootloaderGroup bl_fram_block = {};
volatile At91TransferStates spi_transfer_state = IDLE;

#if USE_FRAM_NON_INTERRUPT_DRV == 0
/* Forward declaration, defined in common source file */
extern void fram_callback(At91SpiBuses bus, At91TransferStates state, void* args);
#endif

#if USE_SIMPLE_BOOTLOADER == 1
void simple_bootloader();
#endif

#endif /* USE_FREERTOS == 0 */

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

    //-------------------------------------------------------------------------
    // Enable I-Cache
    //-------------------------------------------------------------------------
    CP15_Enable_I_Cache();

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
#else
    WDT_start();
    WDT_forceKick();
    /* This call is necessary! Maybe it switches the power supply on? */
    FRAM_start();
#endif /* USE_FREERTOS == 0 */

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

#if USE_SIMPLE_BOOTLOADER == 1
    simple_bootloader();
#else
    initialize_all_iobc_peripherals();
    perform_bootloader_core_operation();
#endif /* USE_SIMPLE_BOOTLOADER == 1 */
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
#endif

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
#if USE_FREERTOS == 1
            vTaskEndScheduler();
#endif
            disable_pit_aic();

            jump_to_sdram_application(0x22000000 - 1024, SDRAM_DESTINATION);
        }
    }
    else {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("CRC field is blank!\n\r");
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */
    }
}

At91TransferStates wait_on_transfer(uint32_t block_cycles, uint32_t* wait_cycles_req) {
#if USE_FREERTOS == 0
    At91TransferStates temp_transfer_state;
    uint32_t wait_cycles = 0;
    while(true) {
        if(spi_transfer_state == SPI_SUCCESS) {
            temp_transfer_state = spi_transfer_state;
            break;
        }
        else if(spi_transfer_state == SPI_OVERRUN_ERROR) {
            temp_transfer_state = spi_transfer_state;
            break;
        }
        else if(wait_cycles == block_cycles) {
            temp_transfer_state = SPI_TIMEOUT;
            break;
        }
        wait_cycles++;
    }
    spi_transfer_state = IDLE;
    if(wait_cycles_req != NULL) {
        *wait_cycles_req = wait_cycles;
    }
    return temp_transfer_state;
#else
    return IDLE;
#endif
}

void initialize_all_iobc_peripherals() {
    //RTT_start();

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
    else {
        result = fram_read_bootloader_block(&bl_info_struct);
        if(result != 0) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
            TRACE_WARNING("initialize_iobc_peripherals: "
                    "FRAM read op for BL block failed with code %d\n\r", retval);
#endif
        }
    }
#else
    memset(&bl_fram_block, 0, sizeof(bl_fram_block));
#if USE_FRAM_NON_INTERRUPT_DRV == 0
    int retval = fram_start_no_os(fram_callback, (void*) &spi_transfer_state,
            AT91C_AIC_PRIOR_HIGHEST - 2);
    if(retval != 0) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("initialize_iobc_peripherals: "
                "Could not start FRAM (No OS), code %d\n\r", retval);
        fram_faulty = true;
#endif
    }
    else {
        /* Start reading the bootloader block with DMA right away */
        retval = fram_no_os_read_bootloader_block(&bl_fram_block);
        if(retval != 0) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
            TRACE_WARNING("initialize_iobc_peripherals: "
                    "FRAM read op for BL block failed with code %d\n\r", retval);
#endif
        }
    }
#else

    int retval = fram_start_no_os_no_interrupt();
    if(retval != 0) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("FRAM initialization failed with code %d\n\r", retval);
#endif
        fram_faulty = true;
    }
    retval = fram_no_os_blocking_read_bootloader_block(&bl_fram_block);
    if(retval != 0) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("FRAM reading BL block failed with code %d\n\r", retval);
#endif
        fram_faulty = true;
    }

#endif /* !USE_FRAM_NON_INTERRUPT_DRV == 0 */

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    print_bl_info();
#endif

#endif /* USE_FREERTOS == 0 */
}


#if USE_FREERTOS == 0

void simple_bootloader() {

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO_WP("-- SOURCE Bootloader v%d.%d --\n\r", BL_VERSION, BL_SUBVERSION);
#endif

    /* We don't need these */
    // initialize_all_iobc_peripherals();
    // setup_timer_interrupt();
    //uint32_t start = get_ms_counter();

    /* Might need to adapt this to kick the watchdog */
    size_t copy_size = PRIMARY_IMAGE_RESERVED_SIZE;
    uint8_t bucket_num = 10;
    size_t bucket_size = copy_size / bucket_num;
    size_t bucket_rest = copy_size % bucket_num;
    size_t offset = 0;
    for(uint8_t idx = 0; idx < bucket_num; idx++) {
        offset = idx * bucket_size;
        memcpy((void*) SDRAM_DESTINATION + offset,
                (const void*) BINARY_BASE_ADDRESS_READ + offset, bucket_size);
        WDT_forceKick();
    }
    offset = bucket_size * bucket_num;
    memcpy((void*) SDRAM_DESTINATION + offset,
            (const void*) BINARY_BASE_ADDRESS_READ + offset, bucket_rest);

//    for(int idx = 0; idx < 100; idx++) {
//        disable_pit_aic();
//    }
#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Jumping to SDRAM application\n\r");
#endif
    /* 139 ms were measured */
    //TRACE_INFO("Measurement: %d ms\n\r", (int) (get_ms_counter() - start));
    jump_to_sdram_application(0x22000000 - 1024, SDRAM_DESTINATION);
}

#endif /* USE_FREERTOS == 0 */


