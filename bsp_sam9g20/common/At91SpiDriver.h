#ifndef BSP_SAM9G20_COMMON_AT91SPIDRIVER_H_
#define BSP_SAM9G20_COMMON_AT91SPIDRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

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

/**
 * Configures the driver and a specific NPCS and also also starts the respective bus if this
 * was not done already.
 * @param bus
 * @param npcs
 * @param spiMode
 * @param frequency
 * @param dlybct
 * @return
 */
int at91_spi_configure_driver(At91SpiBuses bus, At91Npcs npcs,
        SpiModes spiMode, uint32_t frequency, uint32_t dlybct);

/**
 * Configures the non-blocking driver by setting up the required interrupts and
 * enabling them.
 * @param spi_bus
 * @param interrupt_priority
 * @return
 */
int at91_spi_configure_non_blocking_driver(At91SpiBuses spi_bus, uint8_t interrupt_priority);

/**
 * Perform a blocking transfer.
 * @param spi_bus
 * @param npcs
 * @param send_buf
 * @param recv_buf
 * @param transfer_len
 * @return
 */
int at91_spi_blocking_transfer(At91SpiBuses spi_bus, At91Npcs npcs, const uint8_t* send_buf,
        uint8_t* recv_buf, size_t transfer_len);

int at91_spi_non_blocking_transfer(At91SpiBuses spi_bus, At91Npcs npcs, const uint8_t* send_buf,
        uint8_t* recv_buf, size_t transfer_len,
        void (*finish_callback)(At91SpiBuses bus, void* args), void* callback_args);
#ifdef __cplusplus
}
#endif

#endif /* BSP_SAM9G20_COMMON_AT91SPIDRIVER_H_ */
