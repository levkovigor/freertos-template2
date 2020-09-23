#include "SpacePacketBase.h"
#include <string.h>
#include <stdbool.h>

void init_space_packet_header(SpacePacketPointer* packet, bool is_telecommand,
		bool has_secondary_header, uint16_t apid, uint16_t sequence_count) {
	//reset header to zero:
	memset(packet, 0, sizeof(packet->header) );
	//Set TC/TM bit.
	packet->header.packet_id_h = ((is_telecommand? 1 : 0)) << 4;
	//Set secondaryHeader bit
	packet->header.packet_id_h |= ((has_secondary_header? 1 : 0)) << 3;
	set_apid(packet, apid);
	//Always initialize as standalone packets.
	packet->header.sequence_control_h = 0b11000000;
	set_packet_sequence_count(packet, sequence_count);

}

//CCSDS Methods:
uint8_t get_packet_version_number(SpacePacketPointer* packet) {
	return (packet->header.packet_id_h & 0b11100000) >> 5;
}

void set_apid(SpacePacketPointer* packet, uint16_t new_apid) {
	// Use first three bits of new APID,
	// but keep rest of packet id as it was (see specification).
	packet->header.packet_id_h =
			(packet->header.packet_id_h & 0b11111000) |
			( ( new_apid & 0x0700 ) >> 8 );
	packet->header.packet_id_l = ( new_apid & 0x00FF );
}

bool is_telecommand(SpacePacketPointer* packet) {
	return (packet->header.packet_id_h & 0b00010000) >> 4;
}

bool has_secondary_header(SpacePacketPointer* packet) {
	return (packet->header.packet_id_h & 0b00001000) >> 3;
}

uint16_t get_packet_id(SpacePacketPointer* packet) {
	return ((packet->header.packet_id_h) << 8 ) +
			packet->header.packet_id_l;
}

uint16_t get_apid(SpacePacketPointer* packet) {
	return ((packet->header.packet_id_h & 0b00000111) << 8 ) +
			packet->header.packet_id_l;
}

uint16_t get_packet_sequence_control(SpacePacketPointer* packet) {
	return ( (packet->header.sequence_control_h) << 8 )
		+ packet->header.sequence_control_l;
}

uint8_t get_sequence_flags(SpacePacketPointer* packet) {
	return (packet->header.sequence_control_h & 0b11000000) >> 6 ;
}

uint16_t get_packet_sequence_count(SpacePacketPointer* packet) {
	return ( (packet->header.sequence_control_h & 0b00111111) << 8 )
		+ packet->header.sequence_control_l;
}

void set_packet_sequence_count(SpacePacketPointer* packet,
		uint16_t new_count) {
	packet->header.sequence_control_h =
			(packet->header.sequence_control_h & 0b11000000 ) |
			( ( (new_count%LIMIT_SEQUENCE_COUNT) & 0x3F00 ) >> 8 );
	packet->header.sequence_control_l =
			( (new_count%LIMIT_SEQUENCE_COUNT) & 0x00FF );
}

uint16_t get_packet_data_length(SpacePacketPointer* packet) {
	return ( (packet->header.packet_length_h) << 8 )
		+packet->header.packet_length_l;
}

void set_packet_data_length(SpacePacketPointer* packet, uint16_t new_length) {
	packet->header.packet_length_h = ( ( new_length & 0xFF00 ) >> 8 );
	packet->header.packet_length_l = ( new_length & 0x00FF );
}

size_t get_full_space_packet_size(SpacePacketPointer* packet) {
	//+1 is done because size in packet data length field is: size of data field -1
	return get_packet_data_length(packet) + sizeof(packet->header) + 1;
}

uint32_t get_apid_and_sequence_count(SpacePacketPointer* packet) {
	return (get_apid(packet) << 16) + get_packet_sequence_count(packet);
}

uint8_t* get_packet_data(SpacePacketPointer* packet) {
	return &(packet->packet_data);
}

uint8_t* get_whole_data(SpacePacketPointer* packet) {
	return (uint8_t*) packet;
}
