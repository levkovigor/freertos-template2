#ifndef BLLOADER_CORE_RECEIVER_H_
#define BLLOADER_CORE_RECEIVER_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Public definitions */

/* Public functions */
void handle_binary_reception(void);
void configure_usart0(uint32_t baud_rate, uint16_t timeout);
void uart_polling_task();

typedef enum binary_type {
    BOOTLOADER,
    NOR_FLASH,
    SD_CARD_1_SLOT1,
    SD_CARD_1_SLOT2,
    SD_CARD_2_SLOT1,
    SD_CARD_2_SLOT2
} binary_type_t;


typedef struct __attribute__((packed)) lead_packet_data {
    uint8_t binary_type; //!< Type of binary, see respective enum values above.
    uint16_t number_of_packets; //!< Number of packets which will follow
    uint32_t binary_size_; //!< Size of binary which is sent
    uint32_t hamming_code_offset; //!< Offset of the supplied hamming code
} lead_packet_data_t;

#endif /* BLLOADER_CORE_RECEIVER_H_ */
