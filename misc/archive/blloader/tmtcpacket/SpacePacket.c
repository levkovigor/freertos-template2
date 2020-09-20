#include <tmtcpacket/ccsds_header.h>
#include <tmtcpacket/SpacePacket.h>
#include <string.h>

void create_space_packet(SpacePacketPointer* packet, uint16_t packet_data_length,
		bool is_telecommand, uint16_t apid, uint16_t sequence_count) {
	init_space_packet_header(packet, is_telecommand, false, apid, sequence_count);
	set_packet_sequence_count(packet, sequence_count);
	if ( packet_data_length <= sizeof(PACKET_MAX_SIZE) ) {
		set_packet_data_length(packet, packet_data_length);
	} else {
		set_packet_data_length(packet, sizeof(PACKET_MAX_SIZE));
	}
}

//bool SpacePacket::addWholeData( const uint8_t* p_Data, uint32_t packet_size ) {
//	if ( packet_size <= sizeof(this->data) ) {
//		memcpy( &this->localData.byteStream, p_Data, packet_size );
//		return true;
//	} else {
//		return false;
//	}
//}
