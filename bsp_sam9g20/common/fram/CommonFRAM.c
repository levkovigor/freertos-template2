#include "CommonFRAM.h"

uint32_t determine_ham_code_address(SlotType slotType);

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

uint32_t determine_binary_size_address(SlotType slotType) {
    uint32_t address = 0;
    if(slotType == FLASH_SLOT) {
        address = NOR_FLASH_BINARY_SIZE_ADDR;
    }
    else if(slotType == BOOTLOADER_0) {
        address = BOOTLOADER_SIZE_ADDR;
    }
    return address;
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
