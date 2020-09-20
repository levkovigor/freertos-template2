#include <blloader/utility/crc.h>
#include "TmPacketBase.h"
#include <string.h>

TmPacketPointer* initialize_tm_packet_header(uint8_t* data_ptr,
		const size_t max_size, uint16_t apid, uint8_t service,
		uint8_t subservice, uint8_t packet_subcounter,
		uint16_t sequence_count) {
	init_space_packet_header((SpacePacketPointer*)data_ptr, false, true,
			apid, sequence_count);
	TmPacketPointer * tm_packet = (TmPacketPointer*) data_ptr;
	// Set data Field Header:
	// First, set to zero.
	memset(&tm_packet->data_field, 0, sizeof(tm_packet->data_field));
	set_source_data_size(tm_packet, 0);
	/* Set CCSDS_secondary header flag to 0,
	 * version number to 001 and ack to 0000
	 * NOTE: In PUS-C, the PUS Version is 2 and specified for the first 4 bits.
	 * The other 4 bits of the first byte are the spacecraft time
	 * reference status. To change to PUS-C, set 0b00100000 */
	tm_packet->data_field.version_type_ack = 0b00010000;
	tm_packet->data_field.service_type = service;
	tm_packet->data_field.service_subtype = subservice;
	tm_packet->data_field.subcounter = packet_subcounter;
	// Timestamp packet
	// if (checkAndSetStamper()) {
	// 		timeStamper->addTimeStamp(tm_data->data_field.time,
	//				sizeof(tm_data->data_field.time));
	// }
	return tm_packet;
}

tm_packet_returncode_t set_source_data(TmPacketPointer* tm_packet,
        const size_t max_size, uint8_t* source_data, size_t source_size) {
	if(max_size < calculate_full_tm_packet_length(source_size)) {
		return TM_BUFFER_TOO_SMALL;
	}
	memcpy(get_source_data(tm_packet), source_data, source_size);
	set_source_data_size(tm_packet, source_size);
	return TM_PACKET_OK;
}

void set_tm_error_control(TmPacketPointer* tm_packet) {
	uint32_t full_size = get_full_tm_packet_length(tm_packet);
	uint16_t crc = crc16ccitt_default_start_crc(get_tm_data_const(tm_packet),
			full_size - CRC_SIZE);
	uint32_t size = get_source_data_size(tm_packet);
	get_source_data(tm_packet)[size] = (crc & 0XFF00) >> 8;	// CRCH
	get_source_data(tm_packet)[size + 1] = (crc) & 0X00FF; // CRCL
}

const uint8_t* get_tm_data_const(TmPacketPointer* tm_packet) {
	return (const uint8_t*) tm_packet;
}

size_t get_full_tm_packet_length(TmPacketPointer* tm_packet) {
	return get_full_space_packet_size((SpacePacketPointer*) tm_packet);
}
size_t calculate_full_tm_packet_length(size_t source_data_len){
	return sizeof(CCSDSPrimaryHeader) + sizeof(PUSTmDataFieldHeader) +
			source_data_len + CRC_SIZE;
}

void set_source_data_size(TmPacketPointer* tm_packet, uint16_t size) {
	set_packet_data_length((SpacePacketPointer*) tm_packet,
			size + sizeof(PUSTmDataFieldHeader) + CRC_SIZE - 1);
}

uint8_t get_tm_service(TmPacketPointer* tm_packet) {
	return tm_packet->data_field.service_type;
}

uint8_t get_tm_subservice(TmPacketPointer* tm_packet) {
	return tm_packet->data_field.service_subtype;
}

uint8_t* get_source_data(TmPacketPointer* tm_packet) {
	return &tm_packet->data;
}

uint16_t get_source_data_size(TmPacketPointer* tm_packet) {
	return get_packet_data_length((SpacePacketPointer*)tm_packet) -
			sizeof(tm_packet->data_field) - CRC_SIZE + 1;
}

uint16_t get_tm_error_control(TmPacketPointer* tm_packet) {
	uint32_t size = get_source_data_size(tm_packet) + CRC_SIZE;
	uint8_t* p_to_buffer = &tm_packet->data;
	return (p_to_buffer[size - 2] << 8) + p_to_buffer[size - 1];
}

//bool TmPacketBase::checkAndSetStamper() {
//	if (timeStamper == NULL) {
//		timeStamper = objectManager->get<TimeStamperIF>(timeStamperId);
//		if (timeStamper == NULL) {
//			sif::error << "TmPacketBase::checkAndSetStamper: Stamper not found!"
//					<< std::endl;
//			return false;
//		}
//	}
//	return true;
//}
//ReturnValue_t TmPacketBase::getPacketTime(timeval* timestamp) const {
//	uint32_t tempSize = 0;
//	return CCSDSTime::convertFromCcsds(timestamp, tm_data->data_field.time,
//			&tempSize, sizeof(tm_data->data_field.time));
//}
//uint8_t* TmPacketBase::getPacketTimeRaw() const{
//	return tm_data->data_field.time;
//
//}
//uint32_t TmPacketBase::getTimestampSize() const {
//	return sizeof(tm_data->data_field.time);
//}
//TimeStamperIF* TmPacketBase::timeStamper = NULL;
//object_id_t TmPacketBase::timeStamperId = 0;
