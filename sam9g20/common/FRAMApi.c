#include "FRAMApi.h"
#include "CriticalDataBlock.h"
#include <hal/Storage/FRAM.h>

#include <string.h>

/* Private constants */
static const uint32_t BOOTLOADER_HAMMING_ADDR = FRAM_END_ADDR - BOOTLOADER_HAMMING_RESERVED_SIZE;

const uint32_t NOR_FLASH_HAMMING_ADDR = BOOTLOADER_HAMMING_ADDR - NOR_FLASH_HAMMING_RESERVED_SIZE;
const uint32_t SDC0_SLOT0_HAMMING_ADDR = NOR_FLASH_HAMMING_ADDR - NOR_FLASH_HAMMING_RESERVED_SIZE;
const uint32_t SDC0_SLOT1_HAMMING_ADDR =  SDC0_SLOT0_HAMMING_ADDR - NOR_FLASH_HAMMING_RESERVED_SIZE;
const uint32_t SDC1_SLOT0_HAMMING_ADDR = SDC0_SLOT1_HAMMING_ADDR - NOR_FLASH_HAMMING_RESERVED_SIZE;
const uint32_t SDC1_SLOT1_HAMMING_ADDR =  SDC1_SLOT0_HAMMING_ADDR - NOR_FLASH_HAMMING_RESERVED_SIZE;

/* Private functions */
int get_generic_hamming_flag(uint32_t addr, bool* flag_set);
int manipulate_sdc_hamming_flag(uint32_t val, VolumeId volume, SdSlots slot);

/* Implementation */

int read_critical_block(uint8_t* buffer, const size_t max_size) {
    if(max_size < sizeof(CriticalDataBlock)) {
        return -3;
    }

    return FRAM_read((unsigned char*) buffer, CRITICAL_BLOCK_START_ADDR, sizeof(CriticalDataBlock));
}

int write_software_version(uint8_t software_version,
        uint8_t software_subversion, uint8_t sw_subsubversion) {
    uint8_t write_buffer[3] = {software_version, software_subversion, sw_subsubversion};
    return FRAM_writeAndVerify((unsigned char*) write_buffer,
            SOFTWARE_VERSION_ADDR, sizeof(write_buffer));
}

int read_software_version(uint8_t *software_version,
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

int increment_reboot_counter(uint32_t* new_reboot_counter) {
    if(new_reboot_counter == NULL) {
        return -1;
    }

    FRAM_read((unsigned char*) new_reboot_counter,
            REBOOT_COUNTER_ADDR, sizeof(*new_reboot_counter));
    (*new_reboot_counter)++;

    return FRAM_writeAndVerify((unsigned char*) new_reboot_counter,
            REBOOT_COUNTER_ADDR, sizeof(*new_reboot_counter));
}

int read_reboot_counter(uint32_t* reboot_counter) {
    return FRAM_read((unsigned char*) reboot_counter,
            REBOOT_COUNTER_ADDR, sizeof(reboot_counter));
}

int reset_reboot_counter() {
    uint32_t new_counter = 0;
    return FRAM_writeAndVerify((unsigned char*) &new_counter,
            REBOOT_COUNTER_ADDR, sizeof(new_counter));
}

int write_nor_flash_binary_size(size_t binary_size) {
    return FRAM_writeAndVerify((unsigned char*) &binary_size, NOR_FLASH_BINARY_SIZE_ADDR,
            sizeof(binary_size));
}

int read_nor_flash_binary_size(size_t* binary_size) {
    return FRAM_read((unsigned char*) binary_size, NOR_FLASH_BINARY_SIZE_ADDR,
            sizeof(((CriticalDataBlock*)0)->nor_flash_binary_size));
}

int clear_hamming_check_flag() {
    uint32_t cleared_flag = 0;
    return FRAM_writeAndVerify((unsigned char*) &cleared_flag,
            HAMMING_CHECK_FLAG_ADDR, sizeof(cleared_flag));
}

int set_hamming_check_flag() {
    uint32_t set_flag = 1;
    return FRAM_writeAndVerify((unsigned char*) &set_flag,
            HAMMING_CHECK_FLAG_ADDR, sizeof(set_flag));
}

int get_hamming_check_flag(bool* flag_set) {
    return get_generic_hamming_flag(HAMMING_CHECK_FLAG_ADDR, flag_set);
}

int write_nor_flash_hamming_size(size_t hamming_size, bool set_hamming_flag) {
    return FRAM_writeAndVerify((unsigned char*) set_hamming_flag,
            NOR_FLASH_HAMMING_CODE_SIZE_ADDR,
            sizeof(((CriticalDataBlock*)0)->nor_flash_hamming_code_size));
}


int write_nor_flash_hamming_code(uint8_t* hamming_code, size_t current_offset,
        size_t size_to_write) {
    return FRAM_writeAndVerify((unsigned char*) hamming_code + current_offset,
            NOR_FLASH_HAMMING_ADDR, size_to_write);
}

int set_flash_hamming_flag() {
    uint32_t value = 1;
    return FRAM_writeAndVerify((unsigned char*)&value, NOR_FLASH_HAMMING_FLAG_ADDR, 1);
}

int clear_flash_hamming_flag() {
    uint32_t value = 0;
    return FRAM_writeAndVerify((unsigned char*)&value, NOR_FLASH_HAMMING_FLAG_ADDR, 1);
}

int get_flash_hamming_flag(bool* flag_set) {
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

int read_nor_flash_hamming_code(uint8_t *buffer, const size_t max_buffer, size_t* size_read) {
    size_t hamming_code_size = 0;
    int result = FRAM_read((unsigned char*) hamming_code_size,
            NOR_FLASH_HAMMING_CODE_SIZE_ADDR, sizeof(size_t));
    if(result != 0) {
        return result;
    }

    if(hamming_code_size > max_buffer) {
        return -3;
    }

    if(size_read != NULL) {
        *size_read = hamming_code_size;
    }
    return FRAM_read((unsigned char*) buffer, NOR_FLASH_HAMMING_ADDR, hamming_code_size);
}


int read_nor_flash_hamming_size(size_t* hamming_size, bool* hamming_flag_set) {
    if(hamming_size == NULL  ||  hamming_flag_set == NULL) {
        return -3;
    }
    int result = FRAM_read((unsigned char*) hamming_flag_set,
            NOR_FLASH_HAMMING_CODE_SIZE_ADDR,
            sizeof(((CriticalDataBlock*)0)->nor_flash_hamming_code_size));
    if(result != 0) {
        return result;
    }

    return FRAM_read((unsigned char*) hamming_flag_set,
            HAMMING_CHECK_FLAG_ADDR,
            sizeof(((CriticalDataBlock*)0)->use_hamming_flag));
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

int increment_nor_flash_reboot_counter() {
    uint8_t nor_flash_reboot_counter;
    int result = read_nor_flash_reboot_counter(&nor_flash_reboot_counter);
    if(result != 0) {
        return result;
    }
    nor_flash_reboot_counter ++;
    return FRAM_writeAndVerify((unsigned char*) &nor_flash_reboot_counter,
            NOR_FLASH_REBOOT_COUNTER_ADDRESS, sizeof(nor_flash_reboot_counter));
}

int read_nor_flash_reboot_counter(uint8_t *nor_flash_reboot_counter) {
    return FRAM_read((unsigned char*)nor_flash_reboot_counter,
            NOR_FLASH_REBOOT_COUNTER_ADDRESS,
            sizeof(((CriticalDataBlock*)0)->nor_flash_reboot_counter));
}

int reset_nor_flash_reboot_counter() {
    uint8_t new_reboot_counter = 0;
    return FRAM_writeAndVerify((unsigned char*) &new_reboot_counter,
            NOR_FLASH_REBOOT_COUNTER_ADDRESS, sizeof(new_reboot_counter));
}

int increment_sdc0_slot0_reboot_counter() {
    uint8_t sdc1sl1_reboot_counter;
    int result = read_sdc0_slot0_reboot_counter(&sdc1sl1_reboot_counter);
    if(result != 0) {
        return result;
    }
    sdc1sl1_reboot_counter ++;
    return FRAM_writeAndVerify((unsigned char*) &sdc1sl1_reboot_counter,
            SDC1_SL1_REBOOT_COUNTER_ADDR, sizeof(sdc1sl1_reboot_counter));
}

int read_sdc0_slot0_reboot_counter(uint8_t *sdc1sl1_reboot_counter) {
    return FRAM_read((unsigned char*)sdc1sl1_reboot_counter,
            SDC1_SL1_REBOOT_COUNTER_ADDR,
            sizeof(((CriticalDataBlock*)0)->sdc0_image_slot0_reboot_counter));
}

int reset_sdc0_slot0_reboot_counter() {
    uint8_t new_reboot_counter = 0;
    return FRAM_writeAndVerify((unsigned char*) &new_reboot_counter,
            SDC1_SL1_REBOOT_COUNTER_ADDR, sizeof(new_reboot_counter));
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


int update_seconds_since_epoch(uint32_t secondsSinceEpoch) {
    return FRAM_writeAndVerify((unsigned char*) &secondsSinceEpoch,
            SEC_SINCE_EPOCH_ADDR, sizeof(secondsSinceEpoch));
}

int read_seconds_since_epoch(uint32_t *secondsSinceEpoch) {
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

int write_bootloader_hamming_code(const uint8_t *code, size_t size) {
    int result = FRAM_writeAndVerify((unsigned char*) code,
            BOOTLOADER_HAMMING_ADDR, size);
    if(result != 0) {
        return result;
    }
    return FRAM_writeAndVerify((unsigned char*) &size,
            BOOTLOADER_HAMMING_SIZE_ADDR, sizeof(size));
}

int read_bootloader_hamming_code(uint8_t *code, size_t *size) {
    size_t size_to_read = 0;
    int result = FRAM_read((unsigned char*) &size_to_read,
            BOOTLOADER_HAMMING_SIZE_ADDR, sizeof(size_to_read));
    if (result != 0) {
        return result;
    }
    if(size_to_read > 512) {
        return -1;
    }
    else if(size != NULL) {
        *size = size_to_read;
    }
    return FRAM_read(code, BOOTLOADER_HAMMING_ADDR, size_to_read);
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

