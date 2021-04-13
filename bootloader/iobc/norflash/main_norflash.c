#include "main_norflash.h"
#include "../common/iobc_common.h"
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

int increment_reboot_counter_no_os(SlotType slot_type, uint16_t* new_reboot_counter);

#endif /* USE_FREERTOS == 0 */

#if USE_SIMPLE_BOOTLOADER == 1
void simple_bootloader();
#endif

BootSelect determine_boot_select(bool* use_hamming);
bool local_hamming_flag_check(BootSelect boot_select);
void perform_bootloader_check();
void initialize_all_iobc_peripherals();
int copy_sdcard_binary_to_sdram(BootSelect boot_select, bool use_hamming);
void handle_problematic_sdc_copy_result(BootSelect boot_select);
int increment_sdc_loc_reboot_counter(BootSelect boot_select, uint16_t* curr_reboot_counter);
void handle_problematic_norflash_copy_result();
int handle_hamming_code_check(SlotType slotType, size_t image_size, size_t ham_code_size);
int handle_hamming_code_result(int result);

#if BOOTLOADER_VERBOSE_LEVEL >= 1
void print_bl_info();
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

bool fram_faulty = false;
uint32_t start_time = 0;

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
    //setup_timer_interrupt();
    //uint32_t start = get_ms_counter();
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


int perform_iobc_copy_operation_to_sdram() {
    /* The following bootloader sequence was designed by Jakob Meier and implemented by
    Robin Mueller. A graph can be found in the OBSW DDJF document.

    Determine which binary should be copied to SDRAM first.
    First, we check whether a software update needs to be loaded by checking a FRAM flag.
    The SW update volume (SD card 0 or 1) is specified in the FRAM as well.
    After that, we also check the local reboot counter of either the software update
    (SD card slot 1) or the primary flash image.

    If the reboot counter for the SW update is larger than 3, we go to the flash image.
    If the flash image reboot counter is larger than 3, we try the preferred SD card image.
    If the default SD card image reboot counter is larger than 3, we switch the SD card and
    try to boot the image from the other SD card. If the fault counter is larger than 3 here too
    we boot from NOR-Flash without ECC.

    Hamming code checks can be disabled individually for image types, which will lead to a
    boot of that image without a hamming code check.
    If a hamming code checks fails with ECC error or multibit errors, we increment the reboot
    counter in the FRAM and restart the OBC immediately.
     */
    BootSelect boot_select = BOOT_NOR_FLASH;
    int result = 0;
    bool use_hamming = false;

    /* If there are issues with the FRAM, we just boot from flash */
    if(!fram_faulty) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_INFO("Determining boot select..\n\r");
#endif
        boot_select = determine_boot_select(&use_hamming);
    }

    if(use_hamming) {
        use_hamming = local_hamming_flag_check(boot_select);
    }

    if(boot_select == BOOT_NOR_FLASH) {

#if BOOTLOADER_VERBOSE_LEVEL >= 1
        if(use_hamming) {
            TRACE_INFO("Booting from NOR-Flash with hamming code check\n\r");
        }
        else {
            TRACE_INFO("Booting from NOR-Flash without hamming code check\n\r");
        }
#endif
        result = copy_norflash_binary_to_sdram(PRIMARY_IMAGE_RESERVED_SIZE, use_hamming);

        if(result != 0) {
            handle_problematic_norflash_copy_result();
        }
        /* Increment local reboot counter */
#if USE_FREERTOS == 1
        result = fram_increment_img_reboot_counter(FLASH_SLOT, NULL);
#else
        result = increment_reboot_counter_no_os(FLASH_SLOT, NULL);
#endif
    }
    else {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        if(use_hamming) {
            TRACE_INFO("Booting from SD card with hamming code check\n\r");
        }
        else {
            TRACE_INFO("Booting from SD card without hamming code check\n\r");
        }
#endif
        result = copy_sdcard_binary_to_sdram(boot_select, use_hamming);

        if(result != 0) {
            handle_problematic_sdc_copy_result(boot_select);
        }
        result = increment_sdc_loc_reboot_counter(boot_select, NULL);
    }

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    if(result == 0) {
        TRACE_INFO("Copied successfully\n\r");
    }
    else {
        TRACE_INFO("Copied with issues..\n\r");
    }
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

    return result;
}

/**
 * Handles the copy operation from NOR-Flash to SDRAM
 * @param copy_size
 * @return
 *  - 0 on success (jump to SDRAM)
 *  - -1 on failure (do not jump to SDRAM, increment reboot counter and restart)
 */
int copy_norflash_binary_to_sdram(size_t copy_size, bool use_hamming)
{
    int result = 0;
    // Initialize Nor
    //-------------------------------------------------------------------------
    // => the configuration was already done in LowLevelInit()
    // Transfert data from Nor to External RAM
    //-------------------------------------------------------------------------

    /* For the OSless case, we try to read the hamming code in parallel to the memcpy operation */
#if USE_FREERTOS == 0

#if USE_FRAM_NON_INTERRUPT_DRV == 0
    /* This should not happen, we blocked on completion previously */
    if(spi_transfer_state != IDLE) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("FRAM BL block might have failed\n\r");
#endif
        use_hamming = false;
    }
#endif

    if(use_hamming) {
        size_t ham_size = bl_fram_block.nor_flash_hamming_code_size;
        if(ham_size == 0x00 || ham_size == 0xff) {
    #if BOOTLOADER_VERBOSE_LEVEL >= 1
            TRACE_WARNING("Hamming code size might be invalid\n\r");
    #endif
            use_hamming = false;
        }

#if USE_FRAM_NON_INTERRUPT_DRV == 0
        /* Start DMA transfer in background to be run in parallel to the memcpy operation */
        result = fram_no_os_read_ham_code(FLASH_SLOT, hamming_code_buf,
                sizeof(hamming_code_buf), 0, ham_size);
        if(result != 0) {
            use_hamming = false;
        }
#endif
    }
#endif /* USE_FREERTOS == 0 */

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Copying NOR-Flash binary to SDRAM..\n\r");
#endif

#if USE_FREERTOS == 1
    /* This operation takes 100-200 milliseconds if the whole NOR-Flash is
    copied. But the watchdog is running in a separate task with the highest priority
    and we are using a pre-emptive scheduler so this should not be an issue. */
    memcpy((void*) SDRAM_DESTINATION, (const void*) BINARY_BASE_ADDRESS_READ, copy_size);
#else
    /* Now we need to split up the copy operation to kick the watchdog. Watchdog window
    is 1ms to 50ms */
    {
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
    }
#endif /* USE_FREERTOS == 0 */

    /* Now we can perform the hamming code check on the image in the SDRAM */
    if(use_hamming) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_INFO("Performing hamming code ECC check..\n\r");
#endif
        size_t image_size = bl_fram_block.nor_flash_binary_size;
        size_t ham_code_size = bl_fram_block.nor_flash_hamming_code_size;
        if (image_size == 0 || image_size == 0xffffffff) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
            TRACE_WARNING("Flash image size %d invalid\n\r", image_size);
            /* Still jump to binary */
            return 0;
#endif
        }
        else if(ham_code_size == 0 || ham_code_size == 0xffffffff) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
            TRACE_WARNING("Flash hamming code size %d invalid\n\r", image_size);
            /* Still jump to binary */
            return 0;
#endif
        }
        else {
            int check_result = handle_hamming_code_check(FLASH_SLOT, image_size, ham_code_size);
            result = handle_hamming_code_result(check_result);
        }
    }
    return result;
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
