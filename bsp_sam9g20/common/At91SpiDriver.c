#include "At91SpiDriver.h"

#include <board.h>
#include <AT91SAM9G20.h>
#include <at91/peripherals/spi/spi_at91.h>
#include <at91/peripherals/aic/aic.h>
#include <at91/utility/trace.h>

#include <stddef.h>

typedef enum {
DMA,
BLOCKING
} InternalModes;

typedef enum {
WRITE_AND_READ,
WRITE_ONLY,
READ_ONLY
} InternalReception;

void internal_spi_reset(AT91PS_SPI drv, unsigned int id, At91Npcs npcs);
void at91_start_non_blocking_transfer(AT91PS_SPI drv);
int get_drv_handle(At91SpiBuses spi_bus, At91Npcs npcs, AT91PS_SPI* drv, unsigned int* id);
void select_npcs(AT91PS_SPI drv, unsigned int id, At91Npcs npcs);
void spi_irq_handler_bus_0();
void spi_irq_handler_bus_1();
bool generic_spi_interrupt_handler(AT91PS_SPI drv, unsigned int source, At91TransferStates* state);

uint32_t max_block_cycles = 99999;
uint8_t recv_dummy = 0;


typedef struct {
uint8_t dlybs;
uint8_t dlybct;
uint8_t dlybcs;
uint8_t mode_val;
uint32_t frequency;
} CurrentSpiCfg;

AT91PS_SPI current_drv = NULL;
unsigned int current_spi_id = 0;
At91Npcs current_npcs;
InternalModes current_internal_mode;
CurrentSpiCfg current_spi_cfg;

bool bus_0_active = false;
bool bus_1_active = false;

bool bus_0_aic_configured = false;
bool bus_1_aic_configured = false;

volatile bool tx_finished = false;
volatile bool rx_finished = false;
volatile bool write_only = false;
volatile bool read_only = false;

at91_user_callback_t user_callback_bus_0 = NULL;
void* user_args_bus_0 = NULL;
at91_user_callback_t user_callback_bus_1 = NULL;
void* user_args_bus_1 = NULL;

/* Dumy buffer to shift out DMA replies for write only operations */
uint8_t dummy_buf[2][32] = {};
volatile bool current_dummy_buf = 0;

int at91_spi_configure_driver(At91SpiBuses spi_bus, At91Npcs npcs,
        SpiModes spi_mode, uint32_t frequency, uint8_t dlybct, uint8_t dlybs, uint8_t dlybcs) {
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
    if(spi_mode == SPI_MODE_0) {
        mode_val = AT91C_SPI_NCPHA;
    }
    else if(spi_mode == SPI_MODE_2) {
        mode_val = AT91C_SPI_NCPHA | AT91C_SPI_CPOL;
    }
    else if(spi_mode == SPI_MODE_3) {
        mode_val = AT91C_SPI_CPOL;
    }
    drv->SPI_CSR[npcs] = SPI_SCBR(frequency, BOARD_MCK) | mode_val
            | dlybct << 24 |  dlybs << 16;
    current_spi_cfg.dlybcs = dlybcs;
    current_spi_cfg.dlybct = dlybct;
    current_spi_cfg.dlybs = dlybs;
    current_spi_cfg.frequency = frequency;
    current_spi_cfg.mode_val = mode_val;
    return 0;
}


int at91_spi_blocking_transfer_non_interrupt(At91SpiBuses spi_bus, At91Npcs npcs,
        const uint8_t* send_buf, uint8_t* recv_buf, size_t transfer_len) {
    uint32_t block_limit_idx = 0;
    if(send_buf == NULL || transfer_len == 0) {
        return -1;
    }
    if(spi_bus == SPI_BUS_0 && !bus_0_active) {
        return -3;
    }
    if(spi_bus == SPI_BUS_1 && !bus_1_active) {
        return -3;
    }
    AT91PS_SPI drv = NULL;
    unsigned int id = 0;
    int retval = get_drv_handle(spi_bus, npcs, &drv, &id);
    if(retval != 0) {
        return retval;
    }

    uint8_t* recv_ptr = NULL;
    if(recv_buf != NULL) {
        recv_ptr = recv_buf;
    }
    else {
        recv_ptr = dummy_buf[0];
    }

    if(current_internal_mode == DMA) {
        internal_spi_reset(drv, id, npcs);
        current_internal_mode = BLOCKING;
    }

    /* Fixed peripheral select requires reprogramming the mode register */
    select_npcs(drv, id, npcs);

    while ((drv->SPI_SR & AT91C_SPI_TXEMPTY) == 0) {
        block_limit_idx++;
        if(block_limit_idx == max_block_cycles) {
            return -2;
        }
    }
    uint32_t dummy = drv->SPI_RDR;
    (void) dummy;
    for(size_t idx = 0; idx < transfer_len; idx ++) {
        drv->SPI_TDR = *send_buf;
        while ((drv->SPI_SR & AT91C_SPI_RDRF) == 0) {
            block_limit_idx++;
            if(block_limit_idx == max_block_cycles) {
                return -2;
            }
        }
        *recv_ptr = drv->SPI_RDR;
        send_buf++;
        if(recv_buf != NULL) {
            recv_ptr++;
        }
        block_limit_idx = 0;
    }
    block_limit_idx = 0;
    return 0;
}

int at91_spi_configure_non_blocking_driver(At91SpiBuses spi_bus, uint8_t interrupt_priority) {
    AT91PS_SPI drv = NULL;
    unsigned int id = 0;
    if(spi_bus == SPI_BUS_0 && !bus_0_active) {
        return -3;
    }
    if(spi_bus == SPI_BUS_1 && !bus_1_active) {
        return -3;
    }
    int retval = get_drv_handle(spi_bus, NPCS_0, &drv, &id);
    if(retval != 0) {
        return -1;
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

int at91_spi_non_blocking_transfer(At91SpiBuses spi_bus, At91Npcs npcs, const uint8_t *send_buf,
        uint8_t *recv_buf, size_t transfer_len,
        at91_user_callback_t finish_callback, void* callback_args, bool start_immediately) {
    if(send_buf == NULL || transfer_len == 0) {
        return -1;
    }
    if(spi_bus == SPI_BUS_0 && !bus_0_aic_configured) {
        return -3;
    }
    if(spi_bus == SPI_BUS_1 && !bus_1_aic_configured) {
        return -3;
    }

    AT91PS_SPI drv = NULL;
    unsigned int id = 0;
    int retval = get_drv_handle(spi_bus, npcs, &drv, &id);
    if(retval != 0) {
        return retval;
    }

    uint8_t* recv_ptr = NULL;
    if(recv_buf == NULL) {
        write_only = true;
        recv_ptr = dummy_buf[current_dummy_buf];
    }
    else {
        write_only = false;
        recv_ptr = recv_buf;
    }

    if(current_internal_mode == BLOCKING) {
        internal_spi_reset(drv, id, npcs);
        current_internal_mode = DMA;
    }

    /* Fixed peripheral select requires reprogramming the mode register */
    select_npcs(drv, id, npcs);
    if(spi_bus == SPI_BUS_0) {
        user_callback_bus_0 = finish_callback;
        user_args_bus_0 = callback_args;
    }
    else {
        user_callback_bus_1 = finish_callback;
        user_args_bus_1 = callback_args;
    }
    current_drv = drv;
    current_npcs = npcs;
    current_spi_id = id;
    /* Enable all required interrupts sources */
    drv->SPI_IER = AT91C_SPI_TXBUFE | AT91C_SPI_RXBUFF | AT91C_SPI_ENDRX | AT91C_SPI_ENDTX |
            AT91C_SPI_MODF | AT91C_SPI_OVRES;
    if(!write_only) {
        rx_finished = false;
    }
    tx_finished = false;
    /* This driver currently only supports one consecutive transfer.
    DMA pointer and size registers are assigned here. */
    if (drv->SPI_TCR == 0 && drv->SPI_RCR == 0) {
        drv->SPI_TPR = (unsigned int) send_buf;
        drv->SPI_TCR = transfer_len;
        drv->SPI_RPR = (unsigned int) recv_ptr;
        /* Shift out data into dummy buffer */
        if(write_only) {
            drv->SPI_RCR = sizeof(dummy_buf[0]);
            drv->SPI_RNCR = sizeof(dummy_buf[0]);
            current_dummy_buf = !current_dummy_buf;
            drv->SPI_RNPR = (unsigned int) dummy_buf[current_dummy_buf];
        }
        else {
            drv->SPI_RCR = transfer_len;
        }
        if(start_immediately) {
            at91_start_non_blocking_transfer(drv);
        }
    }
    else {
        return -2;
    }

    return 0;
}

void at91_add_second_transfer(const uint8_t* second_send_buf, uint8_t* second_recv_buf,
        size_t transfer_len) {
    if(current_drv != NULL) {
        current_drv->SPI_TNPR = (unsigned int) second_send_buf;
        current_drv->SPI_TNCR = transfer_len;
        if(second_recv_buf != NULL) {
            current_drv->SPI_RNPR = (unsigned int) second_recv_buf;
            current_drv->SPI_RNCR = transfer_len;
        }
        /* Shift out data into dummy buffer */
        else if(current_drv->SPI_RCR == 0) {
            current_drv->SPI_RCR = sizeof(dummy_buf[0]);
            current_drv->SPI_RNCR = sizeof(dummy_buf[0]);
            current_dummy_buf = !current_dummy_buf;
            current_drv->SPI_RNPR = (unsigned int) dummy_buf[current_dummy_buf];
        }
    }
    at91_start_non_blocking_transfer(current_drv);
}

void at91_start_non_blocking_transfer(AT91PS_SPI drv) {
    if(drv != NULL) {
        drv->SPI_PTCR = AT91C_PDC_TXTEN | AT91C_PDC_RXTEN;
        if(current_spi_id != 0) {
            AIC_EnableIT(current_spi_id);
        }
    }
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
    bool error = false;
    if((status & AT91C_SPI_OVRES) == AT91C_SPI_OVRES) {
        if(state != NULL) {
            *state = SPI_OVERRUN_ERROR;
        }
        disable_mask |= AT91C_SPI_OVRES;
        error = true;
    }
    if((status & AT91C_SPI_MODF) == AT91C_SPI_MODF) {
        if(state != NULL) {
            *state = SPI_MODE_ERROR;
        }
        disable_mask |= AT91C_SPI_MODF;
        error = true;
    }
    if((status & AT91C_SPI_ENDTX) == AT91C_SPI_ENDTX) {
        /* In the future, could be extended to fill second bank here to allow multiple
         transfers */
        /* tx_finished = true; */
        disable_mask |= AT91C_SPI_ENDTX;
    }
    if((status & AT91C_SPI_ENDRX) == AT91C_SPI_ENDRX) {
        /* In the future, could be extended to fill second bank here to allow multiple
         transfers */
        /* rx_finished = true; */
        if(write_only) {
            current_dummy_buf = !current_dummy_buf;
            drv->SPI_RNPR = (unsigned int) dummy_buf[current_dummy_buf];
            drv->SPI_RNCR = sizeof(dummy_buf[0]);
        }
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
    if(error) {
        drv->SPI_PTCR = AT91C_PDC_TXTDIS | AT91C_PDC_RXTDIS;
        finish = true;
    }
    else if((write_only && tx_finished) || (!write_only && tx_finished && rx_finished)) {
        if(write_only) {
            drv->SPI_RNCR = 0;
            drv->SPI_RCR = 0;
        }
        drv->SPI_PTCR = AT91C_PDC_TXTDIS | AT91C_PDC_RXTDIS;
        if(state != NULL) {
            *state = SPI_SUCCESS;
        }
        finish = true;
    }

    return finish;
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

int at91_configure_csr(At91SpiBuses bus, At91Npcs npcs,
        SpiModes spi_mode, uint32_t frequency, uint8_t dlybct, uint8_t dlybs, uint8_t dlybcs) {
    if(npcs > 3) {
        return -1;
    }
    AT91PS_SPI drv = NULL;
    unsigned int id = 0;
    int retval = get_drv_handle(bus, npcs, &drv, &id);
    if(retval != 0) {
        return retval;
    }
    uint32_t mode_val = 0;
    if(spi_mode == SPI_MODE_0) {
        mode_val = AT91C_SPI_NCPHA;
    }
    else if(spi_mode == SPI_MODE_2) {
        mode_val = AT91C_SPI_NCPHA | AT91C_SPI_CPOL;
    }
    else if(spi_mode == SPI_MODE_3) {
        mode_val = AT91C_SPI_CPOL;
    }
    drv->SPI_CSR[npcs] = SPI_SCBR(frequency, BOARD_MCK) | mode_val
            | dlybct << 24 |  dlybs << 16;
    current_spi_cfg.dlybcs = dlybcs;
    current_spi_cfg.dlybct = dlybct;
    current_spi_cfg.dlybs = dlybs;
    current_spi_cfg.frequency = frequency;
    current_spi_cfg.mode_val = mode_val;
    return 0;
}

void internal_spi_reset(AT91PS_SPI drv, unsigned int id, At91Npcs npcs) {
    uint32_t mr_cfg = current_spi_cfg.dlybcs << 24 | SPI_PCS(npcs) | AT91C_SPI_MSTR;
    SPI_Configure(drv, id, mr_cfg);
    SPI_Enable(drv);
    drv->SPI_CSR[npcs] = SPI_SCBR(current_spi_cfg.frequency, BOARD_MCK) | current_spi_cfg.mode_val
            | current_spi_cfg.dlybct << 24 |  current_spi_cfg.dlybs << 16;
}

int at91_stop_driver(At91SpiBuses bus) {
    AT91PS_SPI drv = NULL;
    if(bus == SPI_BUS_0) {
        if(bus_0_aic_configured) {
            AIC_DisableIT(AT91C_ID_SPI0);
            bus_0_aic_configured = false;
        }
        bus_0_active = false;
        drv = AT91C_BASE_SPI0;
    }
    if(bus == SPI_BUS_1) {
        if(bus_1_aic_configured) {
            AIC_DisableIT(AT91C_ID_SPI1);
            bus_1_aic_configured = false;
        }
        bus_1_active = false;
        drv = AT91C_BASE_SPI1;
    }
    if(drv != NULL) {
        SPI_Disable(drv);
        // Execute a software reset of the SPI twice
        drv->SPI_CR = AT91C_SPI_SWRST;
        drv->SPI_CR = AT91C_SPI_SWRST;
    }
    return 0;
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
