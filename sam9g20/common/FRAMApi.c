#include "FRAMApi.h"
#include <hal/Storage/FRAM.h>

#include <string.h>

/**
 * This struct will gather all critical data stored on FRAM.
 */
typedef struct __attribute__((__packed__))  _FRAMCriticalData {
    /* Software information */
    uint8_t software_version;
    uint8_t software_subversion;
    uint8_t software_subsubversion;
    uint8_t filler_sw_version;

    /* Reboot information */
    uint32_t reboot_counter;

    /* Second counter */
    uint32_t seconds_since_epoch;

    /* NOR-Flash binary information */
    uint32_t nor_flash_binary_size;
    /* NOR-Flash binary hamming code size */
    size_t nor_flash_hamming_code_size;
    /* This flag determines whether hamming codes will be used to check the binary.
    It is recommended to clear the flag when updating an image and setting the flag after
    the hamming code for this image has been uploaded as well. Otherwise, a new image might
    be checked with an invalid old hamming code. */
    uint32_t nor_flash_use_hamming_flag;
    uint32_t nor_flash_reboot_counter;

    /* SD-Card */

    /* These value will be used on reboot to determine which SD card is the
    default SD card on reboot. 0: None, 1: SD Card 0, 2: SD Card 1 */
    uint32_t preferedSdCard;

    /* Reboot counters SD Card 0 slot 0 */
    uint32_t sdc0_image_slot0_reboot_counter;
    /* Hamming code flag, explanation see above (nor_flash_use_hamming_flag) */
    uint32_t sdc0_image_slot0_use_hamming_flag;
    /* Hamming code size for SD Card 0 slot 0 */
    size_t sdc0_image_slot0_hamming_size;

    /* Reboot counters SD Card Card 0 slot 1 */
    uint32_t sdc0_image_slot1_reboot_counter;
    /* Hamming code flag, explanation see above */
    uint32_t sdc0_image_slot1_use_hamming_flag;
    /* Hamming code size for SD Card 0 slot 1 */
    size_t sdc0_image_slot1_hamming_size;

    /* Reboot counters SD Card 1 slot 0 */
    uint32_t sdc1_image_slot0_reboot_counter;
    /* Hamming code flag, explanation see above */
    uint32_t sdc1_image_slot0_use_hamming_flag;
    /* Hamming code size for SD Card 1 slot 0 */
    size_t sdc1_image_slot0_hamming_size;

    /* Reboot counters SD Card Card 1 slot 1 */
    uint32_t sdc1_image_slot1_reboot_counter;
    /* Hamming code flag, explanation see above */
    uint32_t sdc1_image_slot1_use_hamming_flag;
    /* Hamming code size for SD Card 1 slot 1*/
    size_t sdc1_image_slot1_hamming_size;

    /*
     * Bootloader binary information. Bootloader itself could also be stored
     * in FRAM. Hamming code will be stored in FRAM in any case.
     */
    uint32_t bootloader_faulty;
    uint32_t bootloader_hamming_code_size;

    /* Software update information */
    uint8_t filler_software_update[1];
    uint8_t software_update_available;
    uint8_t software_update_in_slot_0;
    uint8_t software_update_in_slot_1;

    /* Task information */
    uint8_t filler_tasks[2];
    uint16_t number_of_active_tasks;
} FRAMCriticalData;


/* Software information offsets */
static const uint8_t SOFTWARE_VERSION_ADDR =
        offsetof(FRAMCriticalData, software_version);
static const uint8_t SOFTWARE_SUBVERSION_ADDR =
        offsetof(FRAMCriticalData, software_subversion);
static const uint8_t SOFTWARE_SUBSUBVERSION_ADDR =
        offsetof(FRAMCriticalData, software_subsubversion);

/* Reboot info offset */
static const uint32_t REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, reboot_counter);

static const uint32_t BOOTLOADER_HAMMING_SIZE_ADDR =
        offsetof(FRAMCriticalData, bootloader_hamming_code_size);
static const uint32_t BOOTLOADER_FAULTY_ADDRESS =
        offsetof(FRAMCriticalData, bootloader_faulty);

static const uint32_t SEC_SINCE_EPOCH_ADDR =
        offsetof(FRAMCriticalData, seconds_since_epoch);
const uint32_t SOFTWARE_UPDATE_BOOL_ADDR =
        offsetof(FRAMCriticalData, software_update_available);

/* NOR-Flash binary offsets */
const uint32_t NOR_FLASH_BINARY_SIZE_ADDR =
        offsetof(FRAMCriticalData, nor_flash_binary_size);
const uint32_t NOR_FLASH_HAMMING_CODE_SIZE_ADDR =
        offsetof(FRAMCriticalData, nor_flash_hamming_code_size);
const uint32_t NOR_FLASH_HAMMING_SET_ADDR =
        offsetof(FRAMCriticalData, nor_flash_use_hamming_flag);

const uint32_t NOR_FLASH_REBOOT_COUNTER_ADDRESS =
        offsetof(FRAMCriticalData, nor_flash_reboot_counter);


static const uint32_t PREFERED_SD_CARD_ADDR = offsetof(FRAMCriticalData, preferedSdCard);

const uint32_t SDC0_SL0_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc0_image_slot0_reboot_counter);
const uint32_t SDC0_SL0_HAMMING_SIZ_ADDR =
        offsetof(FRAMCriticalData, sdc0_image_slot0_hamming_size);

const uint32_t SDC0_SL1_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc0_image_slot1_reboot_counter);
const uint32_t SDC0_SL1_HAMMING_SIZ_ADDR =
        offsetof(FRAMCriticalData, sdc0_image_slot1_hamming_size);

const uint32_t SDC1_SL0_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc1_image_slot0_reboot_counter);
const uint32_t SDC1_SL0_HAMMING_SIZ_ADDR =
        offsetof(FRAMCriticalData, sdc1_image_slot0_hamming_size);

const uint32_t SDC1_SL1_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc1_image_slot1_reboot_counter);
const uint32_t SDC1_SL1_HAMMING_SIZ_ADDR =
        offsetof(FRAMCriticalData, sdc1_image_slot1_hamming_size);

const uint32_t NUMBER_OF_ACTIVE_TASKS_ADDRESS =
        offsetof(FRAMCriticalData, number_of_active_tasks);

static const uint32_t BOOTLOADER_HAMMING_ADDR = FRAM_END_ADDR - BOOTLOADER_HAMMING_RESERVED_SIZE;

const uint32_t NOR_FLASH_HAMMING_ADDR = BOOTLOADER_HAMMING_ADDR - NOR_FLASH_HAMMING_RESERVED_SIZE;
const uint32_t SDC0_SLOT0_HAMMING_ADDR = NOR_FLASH_HAMMING_ADDR - NOR_FLASH_HAMMING_RESERVED_SIZE;
const uint32_t SDC0_SLOT1_HAMMING_ADDR =  SDC0_SLOT0_HAMMING_ADDR - NOR_FLASH_HAMMING_RESERVED_SIZE;
const uint32_t SDC1_SLOT0_HAMMING_ADDR = SDC0_SLOT1_HAMMING_ADDR - NOR_FLASH_HAMMING_RESERVED_SIZE;
const uint32_t SDC1_SLOT1_HAMMING_ADDR =  SDC1_SLOT0_HAMMING_ADDR - NOR_FLASH_HAMMING_RESERVED_SIZE;

int get_generic_hamming_flag(uint32_t addr, bool* flag_set);

int write_software_version(uint8_t software_version,
        uint8_t software_subversion, uint8_t sw_subsubversion) {
    int result = FRAM_writeAndVerify((unsigned char*) &software_version,
            SOFTWARE_VERSION_ADDR, sizeof(software_version));
    if(result != 0) {
        return result;
    }

    result = FRAM_writeAndVerify((unsigned char*) &software_subversion,
            SOFTWARE_SUBVERSION_ADDR, sizeof(software_subversion));
    if(result != 0) {
        return result;
    }

    return FRAM_writeAndVerify((unsigned char*) &sw_subsubversion,
            SOFTWARE_SUBSUBVERSION_ADDR, sizeof(sw_subsubversion));
}

int read_software_version(uint8_t *software_version,
        uint8_t* software_subversion, uint8_t* sw_subsubversion) {
    int result = FRAM_read((unsigned char*) software_version,
            SOFTWARE_VERSION_ADDR,
            sizeof(((FRAMCriticalData*)0)->software_version));
    if(result != 0) {
        return result;
    }

    result = FRAM_read((unsigned char*) software_subversion,
            SOFTWARE_SUBVERSION_ADDR,
            sizeof(((FRAMCriticalData*)0)->software_subversion));
    if(result != 0) {
        return result;
    }

    return FRAM_read((unsigned char*) sw_subsubversion,
            SOFTWARE_SUBSUBVERSION_ADDR,
            sizeof(((FRAMCriticalData*)0)->software_subsubversion));
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
            sizeof(((FRAMCriticalData*)0)->nor_flash_binary_size));
}

int clear_nor_flash_hamming_flag() {
    uint32_t cleared_flag = 0;
    return FRAM_writeAndVerify((unsigned char*) &cleared_flag,
            NOR_FLASH_HAMMING_SET_ADDR, sizeof(cleared_flag));
}

int set_nor_flash_hamming_flag() {
    uint32_t set_flag = 1;
    return FRAM_writeAndVerify((unsigned char*) &set_flag,
            NOR_FLASH_HAMMING_SET_ADDR, sizeof(set_flag));
}

int get_nor_flash_hamming_flag(bool* flag_set) {
    return get_generic_hamming_flag(NOR_FLASH_HAMMING_SET_ADDR, flag_set);
}

int write_nor_flash_hamming_size(size_t hamming_size, bool set_hamming_flag) {
    return FRAM_writeAndVerify((unsigned char*) set_hamming_flag,
            NOR_FLASH_HAMMING_CODE_SIZE_ADDR,
            sizeof(((FRAMCriticalData*)0)->nor_flash_hamming_code_size));
}


int write_nor_flash_hamming_code(uint8_t* hamming_code, size_t current_offset,
        size_t size_to_write) {
    return FRAM_writeAndVerify((unsigned char*) hamming_code + current_offset,
            NOR_FLASH_HAMMING_ADDR, size_to_write);
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
            sizeof(((FRAMCriticalData*)0)->nor_flash_hamming_code_size));
    if(result != 0) {
        return result;
    }

    return FRAM_read((unsigned char*) hamming_flag_set,
            NOR_FLASH_HAMMING_SET_ADDR,
            sizeof(((FRAMCriticalData*)0)->nor_flash_use_hamming_flag));
}

int set_bootloader_faulty(bool faulty) {
    return FRAM_writeAndVerify((unsigned char*) &faulty,
            BOOTLOADER_FAULTY_ADDRESS,
            sizeof(faulty));
}

int is_bootloader_faulty(bool *faulty) {
    return FRAM_read((unsigned char*)faulty, BOOTLOADER_FAULTY_ADDRESS,
            sizeof(((FRAMCriticalData*)0)->bootloader_faulty));
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
            sizeof(((FRAMCriticalData*)0)->nor_flash_reboot_counter));
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
            sizeof(((FRAMCriticalData*)0)->sdc0_image_slot0_reboot_counter));
}

int reset_sdc0_slot0_reboot_counter() {
    uint8_t new_reboot_counter = 0;
    return FRAM_writeAndVerify((unsigned char*) &new_reboot_counter,
            SDC1_SL1_REBOOT_COUNTER_ADDR, sizeof(new_reboot_counter));
}

int update_seconds_since_epoch(uint32_t secondsSinceEpoch) {
    return FRAM_writeAndVerify((unsigned char*) &secondsSinceEpoch,
            SEC_SINCE_EPOCH_ADDR, sizeof(secondsSinceEpoch));
}

int read_seconds_since_epoch(uint32_t *secondsSinceEpoch) {
    return FRAM_read((unsigned char*) secondsSinceEpoch,
            SEC_SINCE_EPOCH_ADDR,
            sizeof(((FRAMCriticalData*)0)->seconds_since_epoch));
}

int set_prefered_sd_card(VolumeId volumeId) {
    return FRAM_writeAndVerify((unsigned char*) &volumeId,
            PREFERED_SD_CARD_ADDR, sizeof(VolumeId));
}

int get_prefered_sd_card(VolumeId *volumeId) {
    return FRAM_read((unsigned char*) volumeId,
            PREFERED_SD_CARD_ADDR,
            sizeof(((FRAMCriticalData*)0)->preferedSdCard));
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

