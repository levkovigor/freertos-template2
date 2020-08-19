#ifndef TCPACKETCHECK_H_
#define TCPACKETCHECK_H_

#include <tmtcpacket/TcPacketBase.h>

/**
 * This class performs a formal packet check for incoming PUS Telecommand Packets.
 * Currently, it only checks if the APID and CRC are correct.
 * @ingroup tc_distribution
 */

/**
 * Describes the version number a packet must have to pass.
 */
static const uint8_t CCSDS_VERSION_NUMBER = 0;
/**
 * Describes the secondary header a packet must have to pass.
 */
static const uint8_t CCSDS_SECONDARY_HEADER_FLAG = 0;
/**
 * Describes the TC Packet PUS Version Number a packet must have to pass.
 */
static const uint8_t PUS_VERSION_NUMBER = 1;

typedef enum tc_check_retcode {
    TC_CHECK_OK,
    ILLEGAL_APID,
    INCORRECT_PRIMARY_HEADER,
    INCORRECT_CHECKSUM
} tc_check_retcode_t;

/**
 * This is the actual method to formally check a certain Telecommand Packet.
 * The packet's Application Data can not be checked here.
 * @param current_packet The packt to check
 * @return	- @c RETURN_OK on success.
 * 			- @c INCORRECT_CHECKSUM if checksum is invalid.
 * 			- @c ILLEGAL_APID if APID does not match.
 */
tc_check_retcode_t checkPacket(TcPacketPointer* currentPacket);

uint16_t getApid();
void setApid(uint16_t targetApid);

#endif /* TCPACKETCHECK_H_ */
