#include "At91SpiDriver.h"

#include <board.h>
#include <AT91SAM9G20.h>
#include <at91/peripherals/spi/spi_at91.h>

#include <stddef.h>

int get_drv_handle(At91SpiBuses spi_bus, At91Npcs npcs, AT91PS_SPI* drv, unsigned int* id);

int at91_spi_configure_driver(At91SpiBuses spi_bus, At91Npcs npcs, SpiModes spiMode,
        uint32_t frequency, uint32_t dlybct) {
    if(npcs > 3) {
        return -1;
    }
    uint32_t configuration = SPI_PCS(npcs) | AT91C_SPI_PS_VARIABLE | AT91C_SPI_MSTR;
    AT91PS_SPI drv = NULL;
    unsigned int id = 0;
    int retval = get_drv_handle(spi_bus, npcs, &drv, &id);
    if(retval != 0) {
        return retval;
    }
    if(spi_bus == 0) {
        SPI_Configure(drv, id, configuration);
        SPI_Enable(drv);
    }
    else if(spi_bus == 1) {
        SPI_Configure(drv, id, configuration);
        SPI_Enable(drv);
    }
    else {
        return -2;
    }
    uint32_t mode_val = 0;
    if(spiMode == SPI_MODE_0) {
        mode_val = AT91C_SPI_NCPHA;
    }
    else if(spiMode == SPI_MODE_2) {
        mode_val = AT91C_SPI_NCPHA | AT91C_SPI_CPOL;
    }
    else if(spiMode == SPI_MODE_3) {
        mode_val = AT91C_SPI_CPOL;
    }
    drv->SPI_CSR[npcs] = SPI_SCBR(frequency, BOARD_MCK) | mode_val
            | SPI_DLYBCT(dlybct, BOARD_MCK);
    return 0;
}

int at91_spi_blocking_transfer(At91SpiBuses spi_bus, At91Npcs npcs, const uint8_t* send_buf,
        uint8_t* recv_buf, size_t transfer_len) {
    if(send_buf == NULL || recv_buf == NULL) {
        return -1;
    }
    AT91PS_SPI drv = NULL;
    unsigned int id = 0;
    int retval = get_drv_handle(spi_bus, npcs, &drv, &id);
    if(retval != 0) {
        return retval;
    }

    for(size_t idx = 0; idx < transfer_len; idx ++) {
        SPI_Write(drv, npcs, *send_buf);
        send_buf++;
        *recv_buf = SPI_Read(drv);
        recv_buf++;
    }
    return 0;
}

int get_drv_handle(At91SpiBuses spi_bus, At91Npcs npcs, AT91PS_SPI* drv, unsigned int* id) {
    if(drv == NULL || id == NULL) {
        return -1;
    }
    if(spi_bus == 0) {
        *drv = AT91C_BASE_SPI0;
        *id = AT91C_ID_SPI0;
    }
    else if(spi_bus == 1) {
        *drv = AT91C_BASE_SPI1;
        *id = AT91C_ID_SPI1;
    }
    else {
        return -1;
    }

    if(npcs > 3) {
        return -1;
    }
    return 0;
}
