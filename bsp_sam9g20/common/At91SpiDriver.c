#include "At91SpiDriver.h"

#include <board.h>
#include <AT91SAM9G20.h>
#include <at91/peripherals/spi/spi_at91.h>
#include <at91/peripherals/aic/aic.h>
#include <at91/utility/trace.h>

#include <freertos/FreeRTOS.h>
#include <freertos/include/freertos/task.h>

#include <stddef.h>

int get_drv_handle(At91SpiBuses spi_bus, At91Npcs npcs, AT91PS_SPI* drv, unsigned int* id);
void select_npcs(AT91PS_SPI drv, unsigned int id, At91Npcs npcs);
void spi_irq_handler_bus_0();
void spi_irq_handler_bus_1();
bool generic_spi_interrupt_handler(AT91PS_SPI drv, unsigned int sourcea);

bool bus_0_active = false;
bool bus_1_active = false;

bool bus_0_aic_configured = false;
bool bus_1_aic_configured = false;

volatile bool tx_finished = false;
volatile bool rx_finished = false;

void (*user_callback_bus_0) (At91SpiBuses bus, void* args) = NULL;
void* user_args_bus_0 = NULL;
void (*user_callback_bus_1) (At91SpiBuses bus, void* args) = NULL;
void* user_args_bus_1 = NULL;


int at91_spi_configure_driver(At91SpiBuses spi_bus, At91Npcs npcs,
        SpiModes spiMode, uint32_t frequency, uint8_t dlybct, uint8_t dlybs, uint8_t dlybcs) {
    if(npcs > 3) {
        return -1;
    }
    AT91PS_SPI drv = NULL;
    unsigned int id = 0;
    int retval = get_drv_handle(spi_bus, npcs, &drv, &id);
    if(retval != 0) {
        return retval;
    }
    uint32_t init_cfg = dlybcs << 24 | SPI_PCS(npcs) | AT91C_SPI_MSTR;
    if(spi_bus == SPI_BUS_0) {
        SPI_Configure(drv, id, init_cfg);
        SPI_Enable(drv);
        bus_0_active = true;
    }
    else if(spi_bus == SPI_BUS_1) {
        SPI_Configure(drv, id, init_cfg);
        SPI_Enable(drv);
        bus_1_active = true;
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
            | dlybct << 24 |  dlybs << 16;
    return 0;
}


int at91_spi_blocking_transfer(At91SpiBuses spi_bus, At91Npcs npcs, const uint8_t* send_buf,
        uint8_t* recv_buf, size_t transfer_len) {
    if(send_buf == NULL || recv_buf == NULL) {
        return -1;
    }
    if(spi_bus == SPI_BUS_0 && !bus_0_active) {
        return -2;
    }
    if(spi_bus == SPI_BUS_1 && !bus_1_active) {
        return -2;
    }
    AT91PS_SPI drv = NULL;
    unsigned int id = 0;
    int retval = get_drv_handle(spi_bus, npcs, &drv, &id);
    if(retval != 0) {
        return retval;
    }
    select_npcs(drv, id, npcs);

    while ((drv->SPI_SR & AT91C_SPI_TXEMPTY) == 0);
    for(size_t idx = 0; idx < transfer_len; idx ++) {
        drv->SPI_TDR = *send_buf;
        while ((drv->SPI_SR & AT91C_SPI_RDRF) == 0);
        *recv_buf = drv->SPI_RDR;
        send_buf++;
        recv_buf++;
    }
    return 0;
}

int at91_spi_non_blocking_transfer(At91SpiBuses spi_bus, At91Npcs npcs, const uint8_t *send_buf,
        uint8_t *recv_buf, size_t transfer_len,
        void (*finish_callback)(At91SpiBuses bus, void* args), void* callback_args) {
    if(send_buf == NULL || recv_buf == NULL) {
        return -1;
    }
    AT91PS_SPI drv = NULL;
    unsigned int id = 0;
    int retval = get_drv_handle(spi_bus, npcs, &drv, &id);
    if(retval != 0) {
        return retval;
    }
    select_npcs(drv, id, npcs);
    if(spi_bus == SPI_BUS_0) {
        user_callback_bus_0 = finish_callback;
        user_args_bus_0 = callback_args;
    }
    else {
        user_callback_bus_1 = finish_callback;
        user_args_bus_1 = callback_args;
    }

    /* Enable all required interrupts sources */
    drv->SPI_IER = AT91C_SPI_TXBUFE | AT91C_SPI_RXBUFF | AT91C_SPI_ENDRX | AT91C_SPI_ENDTX |
            AT91C_SPI_MODF | AT91C_SPI_OVRES;
    rx_finished = false;
    tx_finished = false;
//    retval = SPI_ReadBuffer(drv, (void*) recv_buf, transfer_len);
//    if (retval == 0) {
//        return -2;
//    }
    retval = SPI_WriteBuffer(drv, (void*) send_buf, transfer_len);
    if (retval == 0) {
        return -2;
    }
    AIC_EnableIT(id);
    return 0;
}

int at91_spi_configure_non_blocking_driver(At91SpiBuses spi_bus, uint8_t interrupt_priority) {
    AT91PS_SPI drv = NULL;
    unsigned int id = 0;
    int retval = get_drv_handle(spi_bus, NPCS_0, &drv, &id);
    if(retval != 0) {
        return retval;
    }
    if(spi_bus == SPI_BUS_0 && !bus_0_aic_configured) {
        AIC_DisableIT(id);
        AIC_ConfigureIT(id, interrupt_priority, spi_irq_handler_bus_0);
        bus_0_aic_configured = true;
    }
    else if(spi_bus == SPI_BUS_1 && !bus_1_aic_configured) {
        AIC_DisableIT(id);
        AIC_ConfigureIT(id, interrupt_priority, spi_irq_handler_bus_1);
        bus_1_aic_configured = true;
    }
    return 0;
}

void spi_irq_handler_bus_0() {
    //TRACE_INFO("Interrupt on bus 0 received!\n\r");
    bool finished = generic_spi_interrupt_handler(AT91C_BASE_SPI0, AT91C_ID_SPI0);
    if(user_callback_bus_0 != NULL && finished) {
        user_callback_bus_0(SPI_BUS_0, user_args_bus_0);
    }
}

void spi_irq_handler_bus_1() {
    //TRACE_INFO("Interrupt on bus 1 received!\n\r");
    bool finished = generic_spi_interrupt_handler(AT91C_BASE_SPI1, AT91C_ID_SPI1);
    if(user_callback_bus_1 != NULL && finished) {
        user_callback_bus_1(SPI_BUS_1, user_args_bus_1);
    }
}

bool generic_spi_interrupt_handler(AT91PS_SPI drv, unsigned int source) {
    uint32_t status = drv->SPI_SR;
    uint32_t disable_mask = 0;
    //TRACE_INFO("Status Register: %u\n\r", (unsigned int) status);
    if((status & AT91C_SPI_OVRES) == AT91C_SPI_OVRES) {
        //TRACE_INFO("Overrun error on SPI bus 0\n\r");
        //AIC_DisableIT(source);
        disable_mask |= AT91C_SPI_OVRES;
    }
    if((status & AT91C_SPI_MODF) == AT91C_SPI_MODF) {
        //TRACE_INFO("Mode fault on SPI bus 0\n\r");
        disable_mask |= AT91C_SPI_MODF;
    }
    if((status & AT91C_SPI_ENDTX) == AT91C_SPI_ENDTX) {
        disable_mask |= AT91C_SPI_ENDTX;
    }
    if((status & AT91C_SPI_ENDRX) == AT91C_SPI_ENDRX) {
        disable_mask |= AT91C_SPI_ENDRX;
    }
    if((status & AT91C_SPI_TXBUFE) == AT91C_SPI_TXBUFE) {
        disable_mask |= AT91C_SPI_TXBUFE;
    }
    if((status & AT91C_SPI_RXBUFF) == AT91C_SPI_RXBUFF) {
        //TRACE_INFO("Receive buffer full on SPI bus 0\n\r");
        disable_mask |= AT91C_SPI_RXBUFF;
    }
    drv->SPI_IDR = disable_mask;
    if(rx_finished && tx_finished) {
        return true;
    }
    return false;
}

int get_drv_handle(At91SpiBuses spi_bus, At91Npcs npcs, AT91PS_SPI* drv, unsigned int* id) {
    if(drv == NULL || id == NULL) {
        return -1;
    }
    if(spi_bus == SPI_BUS_0) {
        *drv = AT91C_BASE_SPI0;
        *id = AT91C_ID_SPI0;
    }
    else if(spi_bus == SPI_BUS_1) {
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

void select_npcs(AT91PS_SPI drv, unsigned int id, At91Npcs npcs) {
    uint32_t configuration = SPI_PCS(npcs) | AT91C_SPI_MSTR;
    drv->SPI_MR = configuration;
}
