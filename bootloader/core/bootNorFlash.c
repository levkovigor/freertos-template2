#include "bootNorFlash.h"
#include <core/watchdog.h>
#include <utility/trace.h>
#include <utility/CRC.h>
#include <string.h>

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

    // Need to test how long copying that much data takes, we might still
    // have to feed the watchdog..
    // For now, we copy in buckets instead of one go.
    uint8_t binary_divisor = 5;
    TRACE_INFO("Copying NOR-Flash binary (%d bytes) from NOR 0x%lx "
            "to SDRAM 0x%09x\n\r", binary_size, BINARY_BASE_ADDRESS_READ,
                (int) BOARD_SDRAM_BASE_ADDRESS);
    // It is assumed that a sanity check of the binary size was already
    // performed. The binary size is extracted from FRAM.
    uint32_t bucket_size = binary_size / binary_divisor;
    size_t last_packet_bytes = binary_size % bucket_size;

    size_t offset = 0;
    for(uint16_t packet_idx = 0; packet_idx < bucket_size; packet_idx++) {
        offset = packet_idx * bucket_size;
        memcpy((void*)BOARD_SDRAM_BASE_ADDRESS + offset,
                (const void *)BINARY_BASE_ADDRESS_READ + offset,
                binary_size);
        uint16_t sdram_crc = crc16ccitt_default_start_crc(
                (void*)BOARD_SDRAM_BASE_ADDRESS + offset,
                bucket_size);
        uint16_t nor_flash_crc =crc16ccitt_default_start_crc(
                (const void *)BINARY_BASE_ADDRESS_READ + offset,
                bucket_size);
        if(sdram_crc != nor_flash_crc) {
            // Should not happen! Try copying again and if that does not work
            // maybe reboot?
        }
        feed_watchdog_if_necessary();
    }

    if(last_packet_bytes > 0) {
        offset = bucket_size * binary_divisor;
        memcpy((void*)BOARD_SDRAM_BASE_ADDRESS + offset,
                (const void *)BINARY_BASE_ADDRESS_READ + offset,
                last_packet_bytes);
        uint16_t sdram_crc = crc16ccitt_default_start_crc(
                (void*)BOARD_SDRAM_BASE_ADDRESS + offset,
                last_packet_bytes);
        uint16_t nor_flash_crc =crc16ccitt_default_start_crc(
                (const void *)BINARY_BASE_ADDRESS_READ + offset,
                last_packet_bytes);
        if(sdram_crc != nor_flash_crc) {
            // Should not happen! Try copying again and if that does not work
            // maybe reboot?
        }
    }
    feed_watchdog_if_necessary();
    return 0;
}
