#include "FRAMApi.h"
#include "CommonFRAM.h"
#include <hal/Storage/FRAM.h>

#include <string.h>



/* Private functions */
int get_generic_hamming_flag(uint32_t addr, bool* flag_set);
int manipulate_sdc_hamming_flag(uint32_t val, VolumeId volume, SdSlots slot);
uint32_t determine_ham_flag_address(SlotType slotType);
uint32_t determine_ham_size_address(SlotType slotType);
uint32_t determine_img_reboot_counter_addr(SlotType slotType);
int determine_ham_code_address_with_sizecheck(SlotType slotType, uint32_t* address,
        size_t size_to_write);
uint32_t determine_ham_code_address(SlotType slotType);


/* Implementation */

int fram_read_critical_block(uint8_t* buffer, const size_t max_size) {
    if(max_size < sizeof(CriticalDataBlock)) {
        return -3;
    }
    return FRAM_read((unsigned char*) buffer, CRITICAL_BLOCK_START_ADDR, sizeof(CriticalDataBlock));
}

int fram_write_software_version(uint8_t software_version,
        uint8_t software_subversion, uint8_t sw_subsubversion) {
    uint8_t write_buffer[3] = {software_version, software_subversion, sw_subsubversion};
    return FRAM_writeAndVerify((unsigned char*) write_buffer,
            SOFTWARE_VERSION_ADDR, sizeof(write_buffer));
}

int fram_read_software_version(uint8_t *software_version,
        uint8_t* software_subversion, uint8_t* sw_subsubversion) {
    if(!software_subversion || !software_subversion || !sw_subsubversion) {
        return -3;
    }

    uint8_t read_buffer[3];
    int result = FRAM_read((unsigned char*) read_buffer,
            SOFTWARE_VERSION_ADDR, 3);
    if(result != 0) {
        return result;
    }

    *software_version = read_buffer[0];
    *software_subversion = read_buffer[1];
    *sw_subsubversion = read_buffer[2];
    return 0;
}

int fram_increment_reboot_counter(uint32_t* new_reboot_counter) {
    if(new_reboot_counter == NULL) {
        return -3;
    }

    FRAM_read((unsigned char*) new_reboot_counter,
            REBOOT_COUNTER_ADDR, sizeof(*new_reboot_counter));
    (*new_reboot_counter)++;

    return FRAM_writeAndVerify((unsigned char*) new_reboot_counter,
            REBOOT_COUNTER_ADDR, sizeof(*new_reboot_counter));
}

int fram_read_reboot_counter(uint32_t* reboot_counter) {
    return FRAM_read((unsigned char*) reboot_counter,
            REBOOT_COUNTER_ADDR, sizeof(reboot_counter));
}

int fram_reset_reboot_counter() {
    uint32_t new_counter = 0;
    return FRAM_writeAndVerify((unsigned char*) &new_counter,
            REBOOT_COUNTER_ADDR, sizeof(new_counter));
}

int fram_get_flash_ham_flag(bool* flag_set) {
    if(!flag_set) {
        return -3;
    }
    bool flag;
    int result = FRAM_read((unsigned char*)&flag, NOR_FLASH_HAMMING_FLAG_ADDR, 1);
    if(result == 0) {
        *flag_set = flag;
    }
    return result;
}

int set_bootloader_faulty(bool faulty) {
    return FRAM_writeAndVerify((unsigned char*) &faulty,
            BOOTLOADER_FAULTY_ADDRESS,
            sizeof(faulty));
}

int is_bootloader_faulty(bool *faulty) {
    return FRAM_read((unsigned char*)faulty, BOOTLOADER_FAULTY_ADDRESS,
            sizeof(((CriticalDataBlock*)0)->bootloader_faulty));
}

int set_sdc_hamming_flag(VolumeId volume, SdSlots slot) {
    uint32_t value = 1;
    return manipulate_sdc_hamming_flag(value, volume, slot);
}

int clear_sdc_hamming_flag(VolumeId volume, SdSlots slot) {
    uint32_t value = 0;
    return manipulate_sdc_hamming_flag(value, volume, slot);
}

int get_sdc_hamming_flag(bool* flag_set, VolumeId volume, SdSlots slot) {
    if(!flag_set) {
        return -3;
    }
    uint32_t flag = 0;
    int result = 0;
    if(volume == SD_CARD_0) {
        if(slot == SDC_SLOT_0) {
            result = FRAM_read((unsigned char*) &flag, SDC0_SL0_HAMMING_FLAG_ADDR, sizeof(flag));
        }
        else {
            result = FRAM_read((unsigned char*) &flag, SDC0_SL1_HAMMING_FLAG_ADDR, sizeof(flag));
        }
    }
    else {
        if(slot == SDC_SLOT_0) {
            result = FRAM_read((unsigned char*) &flag, SDC1_SL0_HAMMING_FLAG_ADDR, sizeof(flag));
        }
        else {
            result = FRAM_read((unsigned char*) &flag, SDC1_SL1_HAMMING_FLAG_ADDR, sizeof(flag));
        }
    }
    if(result == 0) {
        *flag_set = flag;
    }
    return result;
}

int manipulate_sdc_hamming_flag(uint32_t val, VolumeId volume, SdSlots slot) {
    if(volume == SD_CARD_0) {
        if(slot == SDC_SLOT_0) {
            return FRAM_writeAndVerify((unsigned char*) &val, SDC0_SL0_HAMMING_FLAG_ADDR,
                    sizeof(val));
        }
        else {
            return FRAM_writeAndVerify((unsigned char*) &val, SDC0_SL1_HAMMING_FLAG_ADDR,
                    sizeof(val));
        }
    }
    else {
        if(slot == SDC_SLOT_0) {
            return FRAM_writeAndVerify((unsigned char*) &val, SDC1_SL0_HAMMING_FLAG_ADDR,
                    sizeof(val));
        }
        else {
            return FRAM_writeAndVerify((unsigned char*) &val, SDC1_SL1_HAMMING_FLAG_ADDR,
                    sizeof(val));
        }
    }
}


int fram_update_seconds_since_epoch(uint32_t secondsSinceEpoch) {
    return FRAM_writeAndVerify((unsigned char*) &secondsSinceEpoch,
            SEC_SINCE_EPOCH_ADDR, sizeof(secondsSinceEpoch));
}

int fram_read_seconds_since_epoch(uint32_t *secondsSinceEpoch) {
    return FRAM_read((unsigned char*) secondsSinceEpoch,
            SEC_SINCE_EPOCH_ADDR,
            sizeof(((CriticalDataBlock*)0)->seconds_since_epoch));
}

int set_preferred_sd_card(VolumeId volumeId) {
    return FRAM_writeAndVerify((unsigned char*) &volumeId, PREFERRED_SD_CARD_ADDR,
            sizeof(VolumeId));
}

int get_preferred_sd_card(VolumeId *volumeId) {
    return FRAM_read((unsigned char*) volumeId, PREFERRED_SD_CARD_ADDR,
            sizeof(((CriticalDataBlock*)0)->preferredSdCard));
}

int set_to_load_softwareupdate(bool enable, VolumeId volume) {
    bool raw_data[3];
    raw_data[0] = enable;
    if (volume == SD_CARD_0){
        raw_data[1] = true;
    }
    else {
        raw_data[2] = true;
    }
    return FRAM_writeAndVerify((unsigned char*) raw_data,
            SOFTWARE_UPDATE_BOOL_ADDR, 3);
}


// "enable" will tell you if a software update is required
int get_to_load_softwareupdate(bool* enable, VolumeId* volume) {
    if (enable == NULL) {
        return -3;
    }
    bool raw_data[3];
    int result = FRAM_read((unsigned char*) raw_data,
            SOFTWARE_UPDATE_BOOL_ADDR, 3);
    if (result != 0) {
        return result;
    }

    if (raw_data[0] == false) {
        *enable = false;
        return 0;
    }
    if (volume == NULL) {
        return -3;
    }

    if (raw_data[1] == true && raw_data[2] == true) {
        // this should not happen, clear the fields.
        memset(raw_data, 0, 3);
        *enable = false;
        return FRAM_writeAndVerify((unsigned char*) raw_data,
                SOFTWARE_UPDATE_BOOL_ADDR, 3);
    }

    if (raw_data[1] == true) {
        *volume = SD_CARD_0;
        *enable = true;
    }
    else if (raw_data[2] == true) {
        *volume = SD_CARD_1;
        *enable = true;
    }

    // finish by clearing the fields.
    memset(raw_data, 0, 3);
    return FRAM_writeAndVerify((unsigned char*) raw_data,
            SOFTWARE_UPDATE_BOOL_ADDR, 3);
}

int get_generic_hamming_flag(uint32_t addr, bool* flag_set) {
    uint32_t flag_status = 0;
    int result = FRAM_read((unsigned char*) &flag_status, addr, sizeof(flag_status));
    if(result != 0) {
        return result;
    }

    if(flag_set == NULL) {
        return -3;
    }

    if(flag_status > 0) {
        *flag_set = true;
    }
    else {
        *flag_set = false;
    }
    return result;
}

int fram_clear_ham_check_flag() {
    uint32_t cleared_flag = 0;
    return FRAM_writeAndVerify((unsigned char*) &cleared_flag,
            HAMMING_CHECK_FLAG_ADDR, sizeof(cleared_flag));
}

int fram_set_ham_check_flag() {
    uint32_t set_flag = 1;
    return FRAM_writeAndVerify((unsigned char*) &set_flag,
            HAMMING_CHECK_FLAG_ADDR, sizeof(set_flag));
}

int fram_get_ham_check_flag(bool* flag_set) {
    return get_generic_hamming_flag(HAMMING_CHECK_FLAG_ADDR, flag_set);
}

int fram_write_flash_ham_size(size_t hamming_size) {
    return FRAM_writeAndVerify((unsigned char*) &hamming_size, NOR_FLASH_HAMMING_CODE_SIZE_ADDR,
            sizeof(((CriticalDataBlock*)0)->nor_flash_hamming_code_size));
}

int fram_write_binary_size(SlotType slotType, size_t binary_size) {
    uint32_t address = 0;
    if(slotType == FLASH_SLOT) {
        address = NOR_FLASH_BINARY_SIZE_ADDR;
    }
    else if(slotType == BOOTLOADER_0) {
        address = BOOTLOADER_SIZE_ADDR;
    }
    else {
        return -3;
    }

    return FRAM_writeAndVerify((unsigned char*) &binary_size, address, sizeof(binary_size));
}

int fram_read_binary_size(SlotType slotType, size_t *binary_size) {
    if(binary_size == NULL) {
        return -4;
    }

    uint32_t address = 0;
    if(slotType == FLASH_SLOT) {
        address = NOR_FLASH_BINARY_SIZE_ADDR;
    }
    else if(slotType == BOOTLOADER_0) {
        address = BOOTLOADER_SIZE_ADDR;
    }
    else {
        return -4;
    }
    return FRAM_read((unsigned char*) binary_size, address,
            sizeof(((CriticalDataBlock*)0)->nor_flash_binary_size));
}

int fram_set_ham_flag(SlotType slotType) {
    uint32_t address = determine_ham_flag_address(slotType);
    if(address == 0) {
        return -3;
    }
    uint32_t value = 1;
    return FRAM_writeAndVerify((unsigned char*)&value, address, sizeof(uint32_t));
}

int fram_clear_ham_flag(SlotType slotType) {
    uint32_t address = determine_ham_flag_address(slotType);
    if(address == 0) {
        return -3;
    }
    uint32_t value = 0;
    return FRAM_writeAndVerify((unsigned char*)&value, NOR_FLASH_HAMMING_FLAG_ADDR, 1);
}

int fram_get_ham_flag(SlotType slotType, bool *flag_set) {
    if(flag_set == NULL) {
        return -3;
    }
    uint32_t address = determine_ham_flag_address(slotType);
    if(address == 0) {
        return -4;
    }
    uint32_t flag;
    int result = FRAM_read((unsigned char*)&flag, address, sizeof(flag));
    if(result == 0) {
        *flag_set = flag;
    }
    return result;
}

uint32_t determine_ham_flag_address(SlotType slotType) {
    uint32_t address = 0;
    if(slotType == FLASH_SLOT) {
        address = NOR_FLASH_HAMMING_FLAG_ADDR;
    }
    else if(slotType == SDC_0_SL_0) {
        address = SDC0_SL0_HAMMING_FLAG_ADDR;
    }
    else if(slotType == SDC_0_SL_1) {
        address = SDC0_SL1_HAMMING_FLAG_ADDR;
    }
    else if(slotType == SDC_1_SL_0) {
        address = SDC1_SL0_HAMMING_FLAG_ADDR;
    }
    else if(slotType == SDC_1_SL_1) {
        address = SDC1_SL1_HAMMING_FLAG_ADDR;
    }
    return address;
}

int fram_read_img_reboot_counter(SlotType slotType, uint32_t *reboot_counter) {
    if(reboot_counter == NULL) {
        return -3;
    }
    uint32_t address = determine_ham_flag_address(slotType);
    return FRAM_read((unsigned char*) reboot_counter, address,
            sizeof(((CriticalDataBlock*)0)->nor_flash_reboot_counter));
}

int fram_reset_img_reboot_counter(SlotType slotType) {
    uint32_t new_reboot_counter = 0;
    uint32_t address = determine_ham_flag_address(slotType);
    return FRAM_writeAndVerify((unsigned char*) &new_reboot_counter,
            address, sizeof(((CriticalDataBlock*)0)->nor_flash_reboot_counter));
}

int fram_increment_img_reboot_counter(SlotType slotType, uint32_t* new_reboot_counter) {
    uint32_t new_counter_local = 0;
    int result = fram_read_img_reboot_counter(slotType, &new_counter_local);
    if(result != 0) {
        return result;
    }
    new_counter_local++;

    uint32_t address = determine_ham_flag_address(slotType);
    result = FRAM_writeAndVerify((unsigned char*) &new_counter_local,
            address, sizeof(((CriticalDataBlock*)0)->nor_flash_reboot_counter));
    if(result != 0) {
        return result;
    }

    if(new_reboot_counter != NULL) {
        *new_reboot_counter = new_counter_local;
    }
    return result;
}

uint32_t determine_img_reboot_counter_addr(SlotType slotType) {
    uint32_t address = 0;
    if(slotType == FLASH_SLOT) {
        address = NOR_FLASH_REBOOT_COUNTER_ADDRESS;
    }
    else if(slotType == SDC_0_SL_0) {
        address = SDC0_SL0_REBOOT_COUNTER_ADDR;
    }
    else if(slotType == SDC_0_SL_1) {
        address = SDC0_SL1_REBOOT_COUNTER_ADDR;
    }
    else if(slotType == SDC_1_SL_0) {
        address = SDC1_SL0_REBOOT_COUNTER_ADDR;
    }
    else if(slotType == SDC_1_SL_1) {
        address = SDC1_SL1_REBOOT_COUNTER_ADDR;
    }
    return address;
}

int fram_write_ham_size(SlotType slotType, size_t ham_size) {
    uint32_t address = determine_ham_size_address(slotType);
    if(address == 0) {
        return -3;
    }
    return FRAM_writeAndVerify((unsigned char*) &ham_size,
            address, sizeof(((CriticalDataBlock*)0)->nor_flash_hamming_code_size));
}

int fram_read_ham_size(SlotType slotType, size_t *ham_size, bool *ham_flag_set) {
    if(ham_size == NULL) {
        return -3;
    }

    if(ham_flag_set != NULL) {
        bool flag_set = false;
        int result = fram_get_ham_flag(slotType, &flag_set);
        if(result != 0) {
            return result;
        }
        *ham_flag_set = flag_set;
    }

    uint32_t ham_size_addr = determine_ham_flag_address(slotType);
    if(ham_size_addr == 0) {
        return -4;
    }
    return FRAM_read((unsigned char*) ham_size, ham_size_addr,
            sizeof(((CriticalDataBlock*)0)->nor_flash_hamming_code_size));
}

uint32_t determine_ham_size_address(SlotType slotType) {
    uint32_t address = 0;
    if(slotType == FLASH_SLOT) {
        address = NOR_FLASH_HAMMING_CODE_SIZE_ADDR;
    }
    else if(slotType == SDC_0_SL_0) {
        address = SDC0_SL0_HAMMING_SIZE_ADDR;
    }
    else if(slotType == SDC_0_SL_1) {
        address = SDC0_SL1_HAMMING_SIZE_ADDR;
    }
    else if(slotType == SDC_1_SL_0) {
        address = SDC1_SL0_HAMMING_SIZE_ADDR;
    }
    else if(slotType == SDC_1_SL_1) {
        address = SDC1_SL1_HAMMING_SIZE_ADDR;
    }
    else if(slotType == BOOTLOADER_0) {
        address = BOOTLOADER_HAMMING_SIZE_ADDR;
    }
    return address;
}

int fram_write_ham_code(SlotType slotType, uint8_t *buffer, size_t current_offset,
        size_t size_to_write) {
    uint32_t address = 0;
    int result = determine_ham_code_address_with_sizecheck(slotType, &address, size_to_write);
    if(result != 0) {
        return result;
    }
    return FRAM_writeAndVerify((unsigned char*) address + current_offset,
            address, size_to_write);
}

int fram_read_ham_code(SlotType slotType, uint8_t *buffer, const size_t max_buffer,
        size_t current_offset, size_t size_to_read, size_t *size_read) {
    uint32_t address = determine_ham_code_address(slotType);
    if(address == 0) {
        return -4;
    }

    /* Auto-determine size of hamming code */
    if(size_to_read == 0) {
        int result = fram_read_ham_size(slotType, &size_to_read, NULL);
        if(result != 0) {
            return result;
        }
    }

    if(slotType == BOOTLOADER_0) {
        if(size_to_read + current_offset > BOOTLOADER_HAMMING_RESERVED_SIZE) {
            /* Set size to read to remaining size */
            size_to_read = BOOTLOADER_HAMMING_RESERVED_SIZE - current_offset;
        }
    }
    else {
        if(size_to_read + current_offset > IMAGES_HAMMING_RESERVED_SIZE) {
            size_to_read = IMAGES_HAMMING_RESERVED_SIZE - current_offset;
        }
    }

    if(size_to_read > max_buffer) {
        return -5;
    }

    int result = FRAM_read(buffer + current_offset, address, size_to_read);
    if(result != 0) {
        return result;
    }

    if(size_read != NULL) {
        *size_read = size_to_read;
    }
    return result;
}

int determine_ham_code_address_with_sizecheck(SlotType slotType, uint32_t* address,
        size_t size_to_write) {
    uint32_t local_address = determine_ham_code_address(slotType);
    if(local_address == 0) {
        return -4;
    }

    if(slotType == BOOTLOADER_0) {
        if(size_to_write > BOOTLOADER_HAMMING_RESERVED_SIZE) {
            return -5;
        }
    }
    else {
        if(size_to_write > IMAGES_HAMMING_RESERVED_SIZE) {
            return -5;
        }
    }
    *address = local_address;
    return 0;
}

uint32_t determine_ham_code_address(SlotType slotType) {
    uint32_t address = 0;
    if(slotType == FLASH_SLOT) {
        address = NOR_FLASH_HAMMING_ADDR;
    }
    else if(slotType == SDC_0_SL_0) {
        address = SDC0_SLOT0_HAMMING_ADDR;
    }
    else if(slotType == SDC_0_SL_1) {
        address = SDC0_SLOT1_HAMMING_ADDR;
    }
    else if(slotType == SDC_1_SL_0) {
        address = SDC1_SLOT0_HAMMING_ADDR;
    }
    else if(slotType == SDC_1_SL_1) {
        address = SDC1_SLOT1_HAMMING_ADDR;
    }
    else if(slotType == BOOTLOADER_0) {
        address = BOOTLOADER_HAMMING_ADDR;
    }
    return address;
}
