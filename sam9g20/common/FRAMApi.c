#include "FRAMApi.h"
#include <hal/Storage/FRAM.h>

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
