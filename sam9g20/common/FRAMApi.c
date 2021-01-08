#include "FRAMApi.h"
#include <hal/Storage/FRAM.h>

#include <string.h>

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
static const uint32_t NOR_FLASH_BINARY_SIZE_ADDRESS =
		offsetof(FRAMCriticalData, nor_flash_binary_size);
static const uint32_t NOR_FLASH_HAMMING_CODE_OFFSET_ADDRESS =
		offsetof(FRAMCriticalData, nor_flash_hamming_code_size);
static const uint32_t NOR_FLASH_REBOOT_COUNTER_ADDRESS =
        offsetof(FRAMCriticalData, nor_flash_reboot_counter);


static const uint32_t PREFERED_SD_CARD_ADDR =
		offsetof(FRAMCriticalData, preferedSdCard);
static const uint32_t SDC1SL1_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc1sl1_reboot_counter);
const uint32_t SDC1SL2_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc1sl2_reboot_counter);
const uint32_t SDC2SL1_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc2sl1_reboot_counter);
const uint32_t SDC2SL2_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc2sl2_reboot_counter);

const uint32_t NUMBER_OF_ACTIVE_TASKS_ADDRESS =
        offsetof(FRAMCriticalData, number_of_active_tasks);

/** Big blocks at the end of FRAM */
static const uint32_t FRAM_END_ADDR = 0x100000;

// 512 bytes of the upper FRAM will be reserved for the bootloader hamming
// code.
static const size_t BOOTLOADER_HAMMING_RESERVED_SIZE = 512;
#define BOOTLOADER_HAMMING_ADDR FRAM_END_ADDR - \
        BOOTLOADER_HAMMING_RESERVED_SIZE

// 12 kB of the upper FRAM will be reserved for the NOR-Flash binary hamming
// code.
const uint32_t NOR_FLASH_HAMMING_RESERVED_SIZE = 12288;
#define  NOR_FLASH_HAMMING_ADDR BOOTLOADER_HAMMING_ADDR - \
        NOR_FLASH_HAMMING_RESERVED_SIZE

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



int write_nor_flash_binary_info(size_t binary_size, size_t hamming_code_offset) {
	int result = FRAM_writeAndVerify((unsigned char*) &binary_size,
			NOR_FLASH_BINARY_SIZE_ADDRESS, sizeof(binary_size));
	if(result != 0) {
		return result;
	}

	return FRAM_writeAndVerify((unsigned char*) hamming_code_offset,
			NOR_FLASH_HAMMING_CODE_OFFSET_ADDRESS, sizeof(hamming_code_offset));
}

int read_nor_flash_binary_info(size_t* binary_size, size_t* hamming_code_offset) {
	int result = FRAM_read((unsigned char*) binary_size,
			NOR_FLASH_BINARY_SIZE_ADDRESS,
			sizeof(((FRAMCriticalData*)0)->nor_flash_binary_size));
	if(result != 0) {
		return result;
	}

	return FRAM_read((unsigned char*) hamming_code_offset,
			NOR_FLASH_HAMMING_CODE_OFFSET_ADDRESS,
			sizeof(((FRAMCriticalData*)0)->nor_flash_hamming_code_size));
	return 0;
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

int increment_sdc1sl1_reboot_counter() {
    uint8_t sdc1sl1_reboot_counter;
    int result = read_sdc1sl1_reboot_counter(&sdc1sl1_reboot_counter);
    if(result != 0) {
        return result;
    }
    sdc1sl1_reboot_counter ++;
    return FRAM_writeAndVerify((unsigned char*) &sdc1sl1_reboot_counter,
            SDC1SL1_REBOOT_COUNTER_ADDR, sizeof(sdc1sl1_reboot_counter));
}

int read_sdc1sl1_reboot_counter(uint8_t *sdc1sl1_reboot_counter) {
    return FRAM_read((unsigned char*)sdc1sl1_reboot_counter,
            SDC1SL1_REBOOT_COUNTER_ADDR,
            sizeof(((FRAMCriticalData*)0)->sdc1sl1_reboot_counter));
}

int reset_sdc1sl1_reboot_counter() {
    uint8_t new_reboot_counter = 0;
    return FRAM_writeAndVerify((unsigned char*) &new_reboot_counter,
            SDC1SL1_REBOOT_COUNTER_ADDR, sizeof(new_reboot_counter));
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

int set_to_load_softwareupdate(bool slot0) {
	bool raw_data[3];
	raw_data[0] = true;
	if (slot0 == true){
		raw_data[1] = true;
	}
	else {
		raw_data[2] = true;
	}
	return FRAM_writeAndVerify((unsigned char*) raw_data,
			SOFTWARE_UPDATE_BOOL_ADDR, 3);
}

// "yes" tells you if a software update is required
int get_software_to_be_updated(bool* yes, bool* slot0) {
	if (yes == NULL) {
		return -3;
    }
	bool raw_data[3];
	int result = FRAM_read((unsigned char*) raw_data,
			SOFTWARE_UPDATE_BOOL_ADDR, 3);
	if (result != 0) {
		return result;
    }

    if (raw_data[0] == false) {
        *yes = false;
        return 0;
	}
    if (slot0 == NULL) {
    	return -3;
	}

    if (raw_data[1] == true && raw_data[2] == true) {
    	memset(raw_data, 0, 3);
    	*yes = false;
    	return FRAM_writeAndVerify((unsigned char*) raw_data,
    			SOFTWARE_UPDATE_BOOL_ADDR, 3);
	}

    if (raw_data[1] == true) {
		*slot0 = true;
		*yes = true;
    }
    else if (raw_data[2] == true) {
    	*slot0 = false;
    	*yes = true;
    }

	memset(raw_data, 0, 3);
	return FRAM_writeAndVerify((unsigned char*) raw_data,
			SOFTWARE_UPDATE_BOOL_ADDR, 3);
}
