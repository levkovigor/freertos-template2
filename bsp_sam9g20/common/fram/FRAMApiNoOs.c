#include "FRAMApiNoOs.h"
#include "FRAMNoOs.h"

int fram_no_os_read_ham_checks(uint32_t* address, SlotType slotType, const size_t max_buffer,
        size_t current_offset, size_t size_to_read);

/* Defined in common source file */
extern uint32_t determine_img_reboot_counter_addr(SlotType slotType);
extern uint32_t determine_ham_code_address(SlotType slotType);
extern uint32_t determine_ham_size_address(SlotType slotType);
extern uint32_t determine_binary_size_address(SlotType slotType);

int fram_no_os_read_critical_block(uint8_t* buffer, const size_t max_size) {
    if(max_size < sizeof(CriticalDataBlock)) {
        return -3;
    }
    return fram_read_no_os(buffer, CRITICAL_BLOCK_START_ADDR, sizeof(CriticalDataBlock));
}

int fram_no_os_read_bootloader_block_raw(uint8_t* buff, size_t max_size) {
    if(max_size > sizeof(BootloaderGroup)) {
        return -1;
    }
    return fram_read_no_os((unsigned char*) buff, BL_GROUP_ADDR, sizeof(BootloaderGroup));
}

int fram_no_os_read_bootloader_block(BootloaderGroup* bl_info) {
    return fram_read_no_os((unsigned char*) bl_info, BL_GROUP_ADDR, sizeof(BootloaderGroup));
}

int fram_no_os_blocking_read_bootloader_block(BootloaderGroup* bl_info) {
    return fram_read_no_os_blocking((unsigned char*) bl_info,
            BL_GROUP_ADDR, sizeof(BootloaderGroup));
}

int fram_no_os_increment_img_reboot_counter(SlotType slotType, uint16_t new_reboot_counter) {
    uint32_t address = determine_img_reboot_counter_addr(slotType);
    return fram_write_no_os((unsigned char*) &new_reboot_counter,
            address, sizeof(((CriticalDataBlock*)0)->bl_group.nor_flash_reboot_counter));
}

int fram_no_os_blocking_increment_img_reboot_counter(SlotType slotType,
        uint16_t new_reboot_counter) {
    uint32_t address = determine_img_reboot_counter_addr(slotType);
    return fram_write_no_os_blocking((unsigned char*) &new_reboot_counter,
            address, sizeof(((CriticalDataBlock*)0)->bl_group.nor_flash_reboot_counter));
}

int fram_no_os_read_img_reboot_counter(SlotType slotType, uint16_t* reboot_counter) {
    if(reboot_counter == NULL) {
        return -3;
    }
    uint32_t address = determine_img_reboot_counter_addr(slotType);
    return fram_read_no_os((unsigned char*) reboot_counter, address,
            sizeof(((CriticalDataBlock*)0)->bl_group.nor_flash_reboot_counter));
}

int fram_no_os_blocking_read_img_reboot_counter(SlotType slotType, uint16_t* reboot_counter) {
    if(reboot_counter == NULL) {
        return -3;
    }
    uint32_t address = determine_img_reboot_counter_addr(slotType);
    return fram_read_no_os_blocking((unsigned char*) reboot_counter, address,
            sizeof(((CriticalDataBlock*)0)->bl_group.nor_flash_reboot_counter));
}

int fram_no_os_read_binary_size(SlotType slotType, size_t *binary_size) {
    if(binary_size == NULL) {
        return -4;
    }

    uint32_t address = determine_binary_size_address(slotType);
    if(address == 0) {
        return -4;
    }

    return fram_read_no_os((unsigned char*) binary_size, address,
            sizeof(((CriticalDataBlock*)0)->bl_group.nor_flash_binary_size));
}

int fram_no_os_read_ham_size(SlotType slotType, size_t *ham_size) {
    if(ham_size == NULL) {
        return -3;
    }

    uint32_t ham_size_addr = determine_ham_size_address(slotType);
    if(ham_size_addr == 0) {
        return -4;
    }
    return fram_read_no_os((unsigned char*) ham_size, ham_size_addr,
            sizeof(((CriticalDataBlock*)0)->bl_group.nor_flash_hamming_code_size));
}

int fram_no_os_read_ham_code(SlotType slotType, uint8_t* buffer, const size_t max_buffer,
        size_t current_offset, size_t size_to_read) {
    uint32_t address = 0;
    int retval = fram_no_os_read_ham_checks(&address, slotType, max_buffer,
            current_offset, size_to_read);
    if(retval != 0) {
        return retval;
    }

    return fram_read_no_os(buffer + current_offset, address, size_to_read);
}

int fram_no_os_blocking_read_ham_code(SlotType slotType, uint8_t* buffer, const size_t max_buffer,
        size_t current_offset, size_t size_to_read) {
    uint32_t address = 0;
    int retval = fram_no_os_read_ham_checks(&address, slotType, max_buffer,
            current_offset, size_to_read);
    if(retval != 0) {
        return retval;
    }

    return fram_read_no_os_blocking(buffer + current_offset, address, size_to_read);
}

int fram_no_os_read_ham_checks(uint32_t* address, SlotType slotType, const size_t max_buffer,
        size_t current_offset, size_t size_to_read) {
    uint32_t address_loc = determine_ham_code_address(slotType);
    if(address_loc == 0) {
        return -4;
    }

    if(size_to_read == 0) {
        return -4;
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

    if(address != NULL) {
        *address = address_loc;
    }
    return 0;
}
