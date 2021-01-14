#ifndef CRC_CCITT_H_
#define CRC_CCITT_H_

#include <stdint.h>

uint16_t crc16ccitt(uint8_t const input[], uint32_t length,
		uint16_t startingCrc);

uint16_t crc16ccitt_default_start_crc(uint8_t const input[], uint32_t length);

extern const uint16_t crc16ccitt_table[256];

#endif /* CRC_H_ */
