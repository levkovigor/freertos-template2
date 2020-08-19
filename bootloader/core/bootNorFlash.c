#include "bootNorFlash.h"
#include <utility/trace.h>
#include <utility/CRC.h>
#include <string.h>

//------------------------------------------------------------------------------
/// Initialize NOR devices and transfer one or several modules from Nor to the
/// target memory (SRAM/SDRAM).
/// @param pTd    Pointer to transfer descriptor array.
/// @param nbTd  Number of transfer descriptors.
//------------------------------------------------------------------------------
int copy_norflash_binary_to_sdram(char* binary_name, size_t binary_size)
{
    // Initialize Nor
    //-------------------------------------------------------------------------
    // => the configuration was already done in LowLevelInit()

    // Transfert data from Nor to External RAM
    //-------------------------------------------------------------------------

    // TODO: Copy in 255 byte buckets and verify a CRC checksum.
    TRACE_INFO("Copying \"%s\" (%d bytes) from NOR 0x%lx to SDRAM 0x%09x\n\r",
                binary_name, binary_size, BINARY_BASE_ADDRESS_READ,
                (int) BOARD_SDRAM_BASE_ADDRESS);

    uint8_t bucket_size = 255;
    // It is assumed that a sanity check of the binary size was already
    // performed. The binary size is extracted from FRAM.
    size_t last_packet_bytes = binary_size % bucket_size;
    size_t copy_packets_num = binary_size / bucket_size;
    if(last_packet_bytes > 0) {
        copy_packets_num += 1;
    }

    size_t offset = 0;
    for(uint16_t packet_idx = 0; packet_idx < copy_packets_num; packet_idx++) {
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
    }
    return 0;
}
