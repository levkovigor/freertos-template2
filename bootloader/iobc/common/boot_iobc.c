#include "boot_iobc.h"
#include <bootloaderConfig.h>
#include <iobc/norflash/iobc_norflash.h>
#include <iobc/norflash/iobc_boot_sd.h>

#include <sam9g20/common/FRAMApi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <utility/trace.h>
#include <utility/CRC.h>
#include <hal/Drivers/LED.h>
#include <hal/Timing/RTT.h>
#include <string.h>

void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType);

void perform_bootloader_core_operation() {
    int result = perform_iobc_copy_operation_to_sdram();
    if(result != 0) {
        // error
    }

#if DEBUG_IO_LIB == 1
    TRACE_INFO("Jumping to SDRAM application..\n\r");
#endif

    vTaskEndScheduler();
    jump_to_sdram_application();
}

int perform_iobc_copy_operation_to_sdram() {
    // determine which binary should be copied to SDRAM first.
    BootSelect boot_select = BOOT_NOR_FLASH;
    bool enable = false;
    VolumeId volume = SD_CARD_0;
    get_to_load_softwareupdate(&enable, &volume);
    int result = get_to_load_softwareupdate(&enable, &volume);
    if (result != 0) {
        TRACE_ERROR("FRAM could not be read!\n\r");
    }
    else {
        if (enable) {
            if (volume == SD_CARD_0) {
                boot_select = BOOT_SD_CARD_0_UPDATE;
            }
            else {
                boot_select = BOOT_SD_CARD_1_UPDATE;
            }
        }
    }

    if(boot_select == BOOT_NOR_FLASH) {
        result = copy_norflash_binary_to_sdram(OBSW_MAX_SIZE);

        if(result != 0) {
            result = copy_sdcard_binary_to_sdram(BOOT_SD_CARD_0_UPDATE);
        }
    }
    else {
        result = copy_sdcard_binary_to_sdram(boot_select);

        if(result != 0) {
            result = copy_norflash_binary_to_sdram(OBSW_MAX_SIZE);
        }
    }

#if DEBUG_IO_LIB == 1
    if(result == 0) {
        TRACE_INFO("Copied successfully!\n\r");
    }
    else {
        TRACE_ERROR("Copy failed with code %d!\n\r", result);
    }
#endif /* DEBUG_IO_LIB == 1 */

    return result;
}

//------------------------------------------------------------------------------
/// Initialize NOR devices and transfer one or several modules from Nor to the
/// target memory (SRAM/SDRAM).
//------------------------------------------------------------------------------
int copy_norflash_binary_to_sdram(size_t binary_size)
{
    // Initialize Nor
    //-------------------------------------------------------------------------
    // => the configuration was already done in LowLevelInit()
    // Transfert data from Nor to External RAM
    //-------------------------------------------------------------------------

#if DEBUG_IO_LIB == 1
    TRACE_INFO("Copying NOR-Flash binary to SDRAM..\n\r");
#endif

    // This operation takes 100-200 milliseconds if the whole NOR-Flash is
    // copied. But the watchdog is running in a separate task with the highest priority
    // and we are using a pre-emtpive scheduler so this should not be an issue.
    memcpy((void*) SDRAM_DESTINATION, (const void*) BINARY_BASE_ADDRESS_READ, binary_size);

    /* Verify that the binary was copied properly. Ideally, we will also run a hamming
    code check here in the future. */
    /* Check whether a hamming code check is necessary */
    bool hamming_flag = false;
    int result = get_nor_flash_hamming_flag(&hamming_flag);
    if(result != 0) {
#if DEBUG_IO_LIB == 1
        TRACE_WARNING("Could not read FRAM for hamming flag, error code %d!\n\r", result);
#endif
        return 0;
    }
    else if(hamming_flag) {
        /* now we can perform the hamming code check here. We will allocate enough memory in the
        SDRAM to store the hamming code, which is stored in the FRAM */
        uint8_t* hamming_code = malloc(NOR_FLASH_HAMMING_RESERVED_SIZE);
        size_t size_read = 0;
        int result = read_nor_flash_hamming_code(hamming_code,
                NOR_FLASH_HAMMING_RESERVED_SIZE, &size_read);
        if(result != 0) {
#if DEBUG_IO_LIB == 1
            TRACE_WARNING("Could not read hamming code, error code %d!\n\r", result);
#endif
            return 0;
        }

    }

    //    for(int idx = 0; idx < binary_size; idx++) {
    //        if(*(uint8_t*)(SDRAM_DESTINATION + idx) !=
    //                *(uint8_t*)(BINARY_BASE_ADDRESS_READ + idx)) {
    //            TRACE_ERROR("Byte SDRAM %d : %2x\n\r", idx,
    //                    *(uint8_t*)(SDRAM_DESTINATION + idx));
    //            TRACE_ERROR("Byte NORFLASH %d : %2x\n\r", idx,
    //                    *(uint8_t*)(BINARY_BASE_ADDRESS_READ + idx));
    //            return -1;
    //        }
    //    }

    return 0;
}


void idle_loop() {
    uint32_t last_time = RTT_GetTime();
    LED_dark(led_2);
    for(;;) {
        uint32_t curr_time = RTT_GetTime();
        if(curr_time - last_time >= 1) {
#if DEBUG_IO_LIB == 1
            TRACE_INFO("Bootloader idle..\n\r");
#endif
            LED_toggle(led_2);
            last_time = curr_time;
        }
    }
}


/**
 * Used internally to jump to SDRAM.
 * @param jumpAddr
 * @param matchType
 */
void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType)
{
    typedef void (*fctType) (volatile unsigned int, volatile unsigned int);
    void (*pFct) (volatile unsigned int r0_val, volatile unsigned int r1_val);

    pFct = (fctType) jumpAddr;
    pFct(0/*dummy value in r0*/, matchType/*matchType in r1*/);

    while(1);//never reach
}
