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

int at91_spi_configure_driver(At91SpiBuses bus, At91Npcs npcs, SpiModes spiMode, uint32_t frequency,
        uint32_t dlybct);

int at91_spi_blocking_transfer(At91SpiBuses spi_bus, At91Npcs npcs, const uint8_t* send_buf,
        uint8_t* recv_buf, size_t transfer_len);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SAM9G20_COMMON_AT91SPIDRIVER_H_ */
