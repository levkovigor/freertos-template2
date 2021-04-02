#include "At91SpiDriver.h"

#include <board.h>
#include <AT91SAM9G20.h>
#include <at91/peripherals/spi/spi_at91.h>
#include <at91/peripherals/aic/aic.h>
#include <at91/utility/trace.h>

#include <stddef.h>

int get_drv_handle(At91SpiBuses spi_bus, At91Npcs npcs, AT91PS_SPI* drv, unsigned int* id);
void select_npcs(AT91PS_SPI drv, unsigned int id, At91Npcs npcs);
void spi_irq_handler_bus_0();
void spi_irq_handler_bus_1();
bool generic_spi_interrupt_handler(AT91PS_SPI drv, unsigned int source, At91TransferStates* state);

uint32_t max_block_cycles = 99999;

bool bus_0_active = false;
bool bus_1_active = false;

bool bus_0_aic_configured = false;
bool bus_1_aic_configured = false;

volatile bool tx_finished = false;
volatile bool rx_finished = false;

void (*user_callback_bus_0) (At91SpiBuses bus, At91TransferStates state, void* args) = NULL;
void* user_args_bus_0 = NULL;
void (*user_callback_bus_1) (At91SpiBuses bus, At91TransferStates state, void* args) = NULL;
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
    uint32_t block_limit_idx = 0;
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
    /* Fixed peripheral select requires reprogramming the mode register */
    select_npcs(drv, id, npcs);

    while ((drv->SPI_SR & AT91C_SPI_TXEMPTY) == 0) {
        block_limit_idx++;
        if(block_limit_idx == max_block_cycles) {
            return -3;
        }
    }
    for(size_t idx = 0; idx < transfer_len; idx ++) {
        drv->SPI_TDR = *send_buf;
        while ((drv->SPI_SR & AT91C_SPI_RDRF) == 0) {
            block_limit_idx++;
            if(block_limit_idx == max_block_cycles) {
                return -3;
            }
        }
        *recv_buf = drv->SPI_RDR;
        send_buf++;
        recv_buf++;
        block_limit_idx = 0;
    }
    block_limit_idx = 0;
    return 0;
}

int at91_spi_non_blocking_transfer(At91SpiBuses spi_bus, At91Npcs npcs, const uint8_t *send_buf,
        uint8_t *recv_buf, size_t transfer_len,
        void (*finish_callback)(At91SpiBuses bus, At91TransferStates state, void* args),
        void* callback_args) {
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
    /* This driver currently only supports one consecutive transfers.
    DMA pointer and size registers are assigned here. */
    if (drv->SPI_TCR == 0 && drv->SPI_RCR == 0) {
        drv->SPI_TPR = (unsigned int) send_buf;
        drv->SPI_TCR = transfer_len;
        drv->SPI_PTCR = AT91C_PDC_TXTEN;
        drv->SPI_RPR = (unsigned int) recv_buf;
        drv->SPI_RCR = transfer_len;
        drv->SPI_PTCR = AT91C_PDC_RXTEN;
    }
    else {
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
    At91TransferStates transferState;
    bool finished = generic_spi_interrupt_handler(AT91C_BASE_SPI0, AT91C_ID_SPI0, &transferState);
    if(finished) {
        if(user_callback_bus_0 != NULL) {
            user_callback_bus_0(SPI_BUS_0, transferState, user_args_bus_0);
        }
        AIC_DisableIT(AT91C_ID_SPI0);
    }
}

void spi_irq_handler_bus_1() {
    At91TransferStates transferState;
    bool finished = generic_spi_interrupt_handler(AT91C_BASE_SPI1, AT91C_ID_SPI1, &transferState);
    if(finished) {
        if(user_callback_bus_1 != NULL) {
            user_callback_bus_1(SPI_BUS_1, transferState, user_args_bus_1);
        }
        AIC_DisableIT(AT91C_ID_SPI1);
    }
}

bool generic_spi_interrupt_handler(AT91PS_SPI drv, unsigned int source, At91TransferStates* state) {
    uint32_t status = drv->SPI_SR;
    uint32_t disable_mask = 0;
    bool finish = false;
    if((status & AT91C_SPI_OVRES) == AT91C_SPI_OVRES) {
        *state = SPI_OVERRUN_ERROR;
        disable_mask |= AT91C_SPI_OVRES;
        finish = true;
    }
    if((status & AT91C_SPI_MODF) == AT91C_SPI_MODF) {
        *state = SPI_MODE_ERROR;
        disable_mask |= AT91C_SPI_MODF;
        return true;
    }
    if((status & AT91C_SPI_ENDTX) == AT91C_SPI_ENDTX) {
        /* In the future, could be extended to fill second bank here to allow multiple
         transfers */
        tx_finished = true;
        disable_mask |= AT91C_SPI_ENDTX;
    }
    if((status & AT91C_SPI_ENDRX) == AT91C_SPI_ENDRX) {
        /* In the future, could be extended to fill second bank here to allow multiple
         transfers */
        rx_finished = true;
        disable_mask |= AT91C_SPI_ENDRX;
    }
    if((status & AT91C_SPI_TXBUFE) == AT91C_SPI_TXBUFE) {
        tx_finished = true;
        disable_mask |= AT91C_SPI_TXBUFE;
    }
    if((status & AT91C_SPI_RXBUFF) == AT91C_SPI_RXBUFF) {
        rx_finished = true;
        disable_mask |= AT91C_SPI_RXBUFF;
    }
    drv->SPI_IDR = disable_mask;
    if(rx_finished && tx_finished) {
        if(state != NULL) {
            *state = SPI_SUCCESS;
        }
        finish = true;
    }
    return finish;
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
    uint32_t cfg = drv->SPI_MR;
    // Clear PCS field
    cfg &= 0xff00ffff;
    cfg |= SPI_PCS(npcs);
    drv->SPI_MR = cfg;
}

void at91_set_max_block_cycles(uint32_t block_cycles_) {
    max_block_cycles = block_cycles_;
}
