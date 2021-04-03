#include "FRAMNoOs.h"
#include "CommonFRAM.h"
#include <AT91SAM9G20.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const uint32_t FRAM_SPI_SPEED = 8256000;
static const At91SpiBuses FRAM_BUS = SPI_BUS_0;
static const At91Npcs FRAM_NPCS = NPCS_0;
static const SpiModes FRAM_MODE = SPI_MODE_0;

static const uint8_t WRITE_ENB_LATCH = 0x06;
static const uint8_t WRITE_CFG = 0x01;
static const uint8_t WRITE_OP_REG = 0x02;
static const uint8_t READ_OP_REG = 0x03;


at91_user_callback_t user_fram_callback = NULL;
void* callback_user_args = NULL;
volatile uint8_t* alloc_buf = NULL;

uint32_t write_verify_addr = 0;
size_t write_verify_len = 0;
volatile bool write_verify_mode = false;

void internal_fram_callback(At91SpiBuses bus, At91TransferStates state, void* args);
int enable_writes();

int fram_start_no_os(at91_user_callback_t callback, void* callback_args) {
    if(callback == NULL) {
        return -1;
    }

    int retval = at91_spi_configure_driver(FRAM_BUS, FRAM_NPCS, FRAM_MODE, FRAM_SPI_SPEED, 0, 0, 0);
    if(retval != 0) {
        return retval;
    }
    retval = at91_spi_configure_non_blocking_driver(FRAM_BUS, AT91C_AIC_PRIOR_LOWEST + 2);
    if(retval != 0) {
        return retval;
    }


    retval = enable_writes();
    if(retval != 0) {
        return retval;
    }
    uint8_t cfg[2];
    uint8_t dummy[2];
    cfg[0] = WRITE_CFG;
    /* No block protection */
    cfg[1] = 0x0;
    retval = at91_spi_blocking_transfer_non_interrupt(FRAM_BUS, FRAM_NPCS, cfg, dummy, sizeof(cfg));
    if(retval != 0) {
        return -2;
    }
    fram_assign_callback(callback, callback_args);
    return 0;
}

void fram_assign_callback(at91_user_callback_t callback, void* args) {
    user_fram_callback = callback;
    callback_user_args = args;
}

int enable_writes() {
    uint8_t dummy = 0;
    return at91_spi_blocking_transfer_non_interrupt(FRAM_BUS, FRAM_NPCS, &WRITE_ENB_LATCH,
            &dummy, 1);
}

int fram_read_no_os(uint8_t* rec_buf, uint32_t address, size_t len) {
    if(address + len > FRAM_END_ADDR) {
        return -1;
    }
    /* I'd like to avoid dynamic memory allocation but then the driver would require a mode
    where is switches to read-only after the first part of the transfer -> complexity
     */
    size_t total_transfer_len = len + 4;
    uint8_t* write_buf = malloc(total_transfer_len);
    memset(write_buf, 0, total_transfer_len);
    write_buf[0] = READ_OP_REG;
    write_buf[1] = (address >> 16) & 0xff;
    write_buf[2] = (address >> 8) & 0xff;
    write_buf[3] = address & 0xff;
    uint8_t rec_dummy[4];
    int retval = at91_spi_non_blocking_transfer(FRAM_BUS, FRAM_NPCS, write_buf, rec_dummy, 4,
            internal_fram_callback, callback_user_args, false);
    if(retval != 0) {
        free(write_buf);
        return retval;
    }

    at91_add_second_transfer(write_buf + 4 , rec_buf, len);
    alloc_buf = write_buf;
    return 0;
}

int fram_write_no_os(uint8_t* send_buf, uint32_t address, size_t len) {
    if(address + len > FRAM_END_ADDR) {
        return -1;
    }
    int retval = enable_writes();
    if(retval != 0) {
        return retval;
    }
    uint8_t write_buf[4];
    write_buf[0] = WRITE_OP_REG;
    write_buf[1] = (address >> 16) & 0xff;
    write_buf[2] = (address >> 8) & 0xff;
    write_buf[3] = address & 0xff;
    retval = at91_spi_non_blocking_transfer(FRAM_BUS, FRAM_NPCS, write_buf, NULL, 4,
            internal_fram_callback, callback_user_args, false);
    if(retval != 0) {
        return retval;
    }
    at91_add_second_transfer(send_buf + 4 , NULL, len);
    return 0;
}

int fram_stop_no_os() {
    return at91_stop_driver(FRAM_BUS);
}

void internal_fram_callback(At91SpiBuses bus, At91TransferStates state, void* args) {
    if(alloc_buf != NULL) {
        free((void*) alloc_buf);
    }
    user_fram_callback(bus, state, args);
}
