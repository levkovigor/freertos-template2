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
        SpiModes spiMode, uint32_t frequency, uint32_t dlybct_ns, uint32_t dlybs_ns) {
    if(npcs > 3) {
        return -1;
    }
    AT91PS_SPI drv = NULL;
    unsigned int id = 0;
    int retval = get_drv_handle(spi_bus, npcs, &drv, &id);
    if(retval != 0) {
        return retval;
    }
    int32_t init_cfg = 0;
    if(spi_bus == SPI_BUS_0 && !bus_0_active) {
        SPI_Configure(drv, id, init_cfg);
        SPI_Enable(drv);
        bus_0_active = true;
    }
    else if(spi_bus == SPI_BUS_1 && !bus_1_active) {
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
            | SPI_DLYBCT(dlybct_ns, BOARD_MCK) |  SPI_DLYBS(dlybs_ns, BOARD_MCK);
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

    for(size_t idx = 0; idx < transfer_len; idx ++) {
        SPI_Write(drv, npcs, *send_buf);
        send_buf++;
        *recv_buf = SPI_Read(drv);
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
    //TRACE_INFO("Status Register: %u\n\r", (unsigned int) status);
    if((status & AT91C_SPI_OVRES) == AT91C_SPI_OVRES) {
        //TRACE_INFO("Overrun error on SPI bus 0\n\r");
        //AIC_DisableIT(source);
        return true;
    }
    if((status & AT91C_SPI_MODF) == AT91C_SPI_MODF) {
        //TRACE_INFO("Mode fault on SPI bus 0\n\r");
    }
    if((status & AT91C_SPI_TXBUFE) == AT91C_SPI_TXBUFE) {
       // TRACE_INFO("Transmit buffer empty on SPI bus 0\n\r");
        tx_finished = true;
        drv->SPI_IDR = AT91C_SPI_TXBUFE;
    }
    if((status & AT91C_SPI_RXBUFF) == AT91C_SPI_RXBUFF) {
        //TRACE_INFO("Receive buffer full on SPI bus 0\n\r");
        rx_finished = true;
        drv->SPI_IDR = AT91C_SPI_RXBUFF;
    }
    if(rx_finished && tx_finished) {
        return true;
    }
    return false;
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

void select_npcs(AT91PS_SPI drv, unsigned int id, At91Npcs npcs) {
    uint32_t configuration = SPI_PCS(npcs) | AT91C_SPI_PS_VARIABLE | AT91C_SPI_MSTR;
    drv->SPI_MR = configuration;
}
