#ifndef BLLOADER_MAIN_H_
#define BLLOADER_MAIN_H_

#include <stdint.h>

/* Packet service */
const uint8_t PACKET_SUBSERVICE = 6;

/* Lead packet subservice */
const uint8_t LEAD_PACKET_SUBSERVICE = 128;
/* Binary packet subservice */
const uint8_t BIN_PACKET_SUBSERVICE = 129;
/* Tail packet subservice */
const uint8_t TAIL_PACKET_SUBSERVICE = 130;

#endif /* BLLOADER_MAIN_H_ */
