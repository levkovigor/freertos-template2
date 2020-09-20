#include <blloader/utility/crc.h>
#include "TcPacketCheck.h"
#include <tmtcpacket/SpacePacketBase.h>
#include <stdbool.h>

/**
 * The packet id each correct packet should have.
 * It is composed of the APID and some static fields.
 */
static uint16_t apid;

tc_check_retcode_t checkPacket(TcPacketPointer* currentPacket) {
    SpacePacketPointer* space_packet = (SpacePacketPointer*) currentPacket;
	uint16_t calculatedCrc = crc16ccitt_default_start_crc(
	        get_whole_data(space_packet),
	        get_full_tc_packet_size(currentPacket));
	if (calculatedCrc != 0) {
		return INCORRECT_CHECKSUM;
	}

	bool condition = (has_secondary_header(space_packet)) ||
	        (!is_telecommand(space_packet) ||
	                !has_secondary_header(space_packet) );
	if (condition) {
	    return INCORRECT_PRIMARY_HEADER;
	}

	if (get_apid(space_packet) != apid)
		return ILLEGAL_APID;

	return TC_CHECK_OK;
}

void setApid(uint16_t targetApid) {
    apid = targetApid;
}
uint16_t getApid() {
	return apid;
}
