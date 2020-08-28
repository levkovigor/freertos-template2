#include "FRAMApi.h"
#include <hal/Storage/FRAM.h>

int increment_reboot_counter() {
	uint16_t current_counter = 0;
	int result = read_reboot_counter(&current_counter);
	if(result != 0) {
		return result;
	}

	current_counter++;
	return FRAM_writeAndVerify((unsigned char*) &current_counter,
			REBOOT_COUNTER_ADDR, sizeof(current_counter));
}

int read_reboot_counter(uint16_t* reboot_counter) {
	return FRAM_read((unsigned char*) reboot_counter,
			REBOOT_COUNTER_ADDR, sizeof(reboot_counter));
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
			sizeof(((FRAMCriticalData*)0)->nor_flash_hamming_code_offset));
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

int update_seconds_since_epoch(uint64_t secondsSinceEpoch) {
	return FRAM_writeAndVerify((unsigned char*) &secondsSinceEpoch,
			SEC_SINCE_EPOCH_ADDR, sizeof(secondsSinceEpoch));
}

int read_seconds_since_epoch(uint64_t *secondsSinceEpoch) {
	return FRAM_read((unsigned char*) secondsSinceEpoch,
			SEC_SINCE_EPOCH_ADDR,
			sizeof(((FRAMCriticalData*)0)->seconds_since_epoch));
}
