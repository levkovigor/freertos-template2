#include <blloader/utility/crc.h>
#include "TcPacketBase.h"
#include <string.h>

TcPacketPointer* initialize_tc_packet_header(uint8_t* data_ptr,
		const size_t max_size, uint16_t apid, uint16_t sequence_count,
		uint8_t ack, uint8_t service, uint8_t subservice) {
	if(max_size < TC_PACKET_MIN_SIZE) {
		return NULL;
	}
	TcPacketPointer* tc_packet = (TcPacketPointer*) data_ptr;
 	init_space_packet_header((SpacePacketPointer*)tc_packet, true, true,
 			apid, sequence_count);
	memset(&tc_packet->data_field, 0, sizeof(tc_packet->data_field));
	set_packet_data_length((SpacePacketPointer*) tc_packet,
			sizeof(PUSTcDataFieldHeader) + CRC_SIZE - 1);
	/* Data Field Header:
	 * Set CCSDS_secondary_header_flag to 0, version number to 001
	 * and ack to 0000 */
	tc_packet->data_field.version_type_ack = 0b00010000;
	tc_packet->data_field.version_type_ack |= (ack & 0x0F);
	tc_packet->data_field.service_type = service;
	tc_packet->data_field.service_subtype = subservice;
	return tc_packet;
}

ReturnValue_t set_app_data(TcPacketPointer* tc_packet, const size_t max_size,
		uint8_t * app_data, uint16_t data_len) {
	if(max_size < calculate_full_tc_packet_length(data_len)) {
		return TC_BUFFER_TOO_SMALL;
	}
	memcpy(&tc_packet->app_data, app_data, data_len);
	set_packet_data_length((SpacePacketPointer*)tc_packet, data_len +
				sizeof(PUSTcDataFieldHeader) + CRC_SIZE - 1);
	return RETURN_OK;
}

void set_tc_error_control(TcPacketPointer* tc_packet) {
	uint32_t full_size = get_full_tc_packet_size(tc_packet);
	uint16_t crc = crc16ccitt_default_start_crc(get_whole_data(
			(SpacePacketPointer*)tc_packet), full_size - CRC_SIZE);
	size_t size = get_application_data_size(tc_packet);
	(&tc_packet->app_data)[size] = (crc & 0XFF00) >> 8;	// CRCH
	(&tc_packet->app_data)[size + 1] = (crc) & 0X00FF; 		// CRCL
}

const uint8_t * get_tc_data_const(TcPacketPointer* tc_packet) {
	return (const uint8_t*) tc_packet;
}

size_t calculate_full_tc_packet_length(size_t app_data_len) {
	return sizeof(CCSDSPrimaryHeader) + sizeof(PUSTcDataFieldHeader) +
			app_data_len + CRC_SIZE;
}

size_t get_full_tc_packet_size(TcPacketPointer* tc_packet) {
	return get_full_space_packet_size((SpacePacketPointer*) tc_packet);
}

uint8_t get_tc_service(TcPacketPointer* tc_packet) {
	return tc_packet->data_field.service_type;
}

uint8_t get_tc_subservice(TcPacketPointer* tc_packet) {
	return tc_packet->data_field.service_subtype;
}

uint8_t get_tc_acknowledge_flags(TcPacketPointer* tc_packet) {
	return tc_packet->data_field.version_type_ack & 0b00001111;
}

const uint8_t* get_application_data(TcPacketPointer* tc_packet) {
	return &tc_packet->app_data;
}

uint16_t get_application_data_size(TcPacketPointer* tc_packet) {
	return get_packet_data_length((SpacePacketPointer*)tc_packet) -
			sizeof(tc_packet->data_field) - CRC_SIZE + 1;
}

uint16_t get_tc_error_control(TcPacketPointer* tc_packet) {
	uint16_t size = get_application_data_size(tc_packet) + CRC_SIZE;
	uint8_t* p_to_buffer = &tc_packet->app_data;
	return (p_to_buffer[size - 2] << 8) + p_to_buffer[size - 1];
}

uint8_t get_secondary_header_flag(TcPacketPointer* tc_packet) {
	return (tc_packet->data_field.version_type_ack & 0b10000000) >> 7;
}

uint8_t get_pus_version_number(TcPacketPointer* tc_packet) {
	return (tc_packet->data_field.version_type_ack & 0b01110000) >> 4;
}
