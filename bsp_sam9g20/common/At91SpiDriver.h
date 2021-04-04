#ifndef BSP_SAM9G20_COMMON_AT91SPIDRIVER_H_
#define BSP_SAM9G20_COMMON_AT91SPIDRIVER_H_

/**
 * Implemented to allow convenient use of SPI without requiring FreeRTOS.
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    SPI_MODE_0,
    SPI_MODE_1,
    SPI_MODE_2,
    SPI_MODE_3
} SpiModes;

typedef enum {
    SPI_BUS_0,
    SPI_BUS_1
} At91SpiBuses;

typedef enum {
    NPCS_0,
    NPCS_1,
    NPCS_2,
    NPCS_3
} At91Npcs;

typedef enum {
    IDLE,
    SPI_SUCCESS,
    SPI_OVERRUN_ERROR,
    SPI_MODE_ERROR
} At91TransferStates;

typedef void (*at91_user_callback_t) (At91SpiBuses bus, At91TransferStates state, void* args);

/**
 * Configures the driver and a specific NPCS and also also starts the respective bus if this
 * was not done already.
 * @param bus
 * @param npcs
 * @param spiMode
 * @param frequency
 * @param dlybct Delay between consecutive transfers, see p.407 SAM9G20 datasheet
 * @param dlybs Delay to first clock transition, see p.407 SAM9G20 datasheet
 * @param dlybcs Delay between chip selects.
 * @return -1 for invalid input
 */
int at91_spi_configure_driver(At91SpiBuses bus, At91Npcs npcs,
        SpiModes spiMode, uint32_t frequency, uint8_t dlybct, uint8_t dlybs, uint8_t dlybcs);

/**
 * Configures the non-blocking driver by setting up the required interrupts and
 * enabling them. Still requires a previous call to #at91_spi_configure_driver
 * @param spi_bus
 * @param interrupt_priority
 * @return -3 if a call to #at91_spi_configure_driver is missing, -1 for invalid input.
 */
int at91_spi_configure_non_blocking_driver(At91SpiBuses spi_bus, uint8_t interrupt_priority);

/**
 * Perform a blocking transfer. It is discouraged to call this in an environment with
 * a pre-emptive scheduler because a task preemption can mess with the timing
 * requirements for the peripheral if no interrupts are used.
 * @param spi_bus
 * @param npcs
 * @param send_buf
 * @param recv_buf      Can be NULL if reply is not required
 * @param transfer_len
 * @return -3 if a call to #at91_spi_configure_driver is missing, -1 for invalid input,
 * -2 if a timeout occurs because the maximum blocking cycles are reached.
 */
int at91_spi_blocking_transfer_non_interrupt(At91SpiBuses spi_bus, At91Npcs npcs,
        const uint8_t* send_buf, uint8_t* recv_buf, size_t transfer_len);

/**
 * Set maximum number of block cycles to avoid deadlocks when not using an OS.
 * @param block_cycles
 * @return
 */
void at91_set_max_block_cycles(uint32_t block_cycles);

/**
 * Initiate non-blocking transfer, using DMA.
 * A second transfer can be queue directly by staggering the start of the transfer and
 * calling #at91_add_second_transfer.
 * @param spi_bus
 * @param npcs
 * @param send_buf
 * @param recv_buf              Can be NULL if reply is not required, reply will be shifter
 *                              into internal dummy buffer
 * @param transfer_len
 * @param finish_callback
 * @param callback_args         Arguments to be passed to the callback.
 * @param start_immediately     Start can be staggered to add second transfer.
 * @return
 *  -1 for invalid input, -2 if another transfer is ongoing, -3 if a call to
 *  #at91_spi_configure_non_blocking_driver is missing.
 */
int at91_spi_non_blocking_transfer(At91SpiBuses spi_bus, At91Npcs npcs, const uint8_t* send_buf,
        uint8_t* recv_buf, size_t transfer_len,
        at91_user_callback_t callback, void* callback_args, bool start_immediately);

/**
 * Add a second transfer which will be executed directly after the first transfer.
 * This function will also start the transfer.
 * @param second_send_buf   Buffer to be sent immediately after first transfer. Can be NULL
 *                          to only send zeros
 * @param second_recv_buf   Buffer to write to immediately after first transfer. Can be null
 *                          to shift reply into internal dummy buffer
 * @param transfer_len
 * @return
 */
void at91_add_second_transfer(const uint8_t* second_send_buf, uint8_t* second_recv_buf,
        size_t transfer_len);

/**
 * Configures the SPI properties for single slave selects without resetting and enabling
 * the SPI peripheral.
 * @param bus
 * @param npcs
 * @param spiMode
 * @param frequency
 * @param dlybct
 * @param dlybs
 * @param dlybcs
 * @return
 */
int at91_configure_csr(At91SpiBuses bus, At91Npcs npcs,
        SpiModes spi_mode, uint32_t frequency, uint8_t dlybct, uint8_t dlybs, uint8_t dlybcs);

/**
 * Stop the driver, disabling AIC interrupts where applicable, deactivating the SPI peripheral
 * and resetting it.
 * @return
 */
int at91_stop_driver(At91SpiBuses bus);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SAM9G20_COMMON_AT91SPIDRIVER_H_ */
