#include "FRAMApiNoOs.h"
#include "FRAMNoOs.h"
#include "CommonFRAM.h"

int fram_no_os_read_critical_block(uint8_t* buffer, const size_t max_size) {
    if(max_size < sizeof(CriticalDataBlock)) {
        return -3;
    }
    return fram_read_no_os(buffer, CRITICAL_BLOCK_START_ADDR, sizeof(CriticalDataBlock));
}

int fram_read_bootloader_block_raw(uint8_t* buff, size_t max_size) {
    if(max_size > sizeof(BootloaderGroup)) {
        return -1;
    }
    return fram_read_no_os((unsigned char*) buff, BL_GROUP_ADDR, sizeof(BootloaderGroup));
}

int fram_read_bootloader_block(BootloaderGroup* bl_info) {
    return fram_read_no_os((unsigned char*) bl_info, BL_GROUP_ADDR, sizeof(BootloaderGroup));
}
