/**
 * This file contains basic functions to create an ECSS PUS Telecommand packet
 * and all related interface functions.
 *
 * The class provides methods to handle the standardized entries of the
 * PUS TC Packet Data Field Header.
 * It does not contain the packet data itself but a pointer to the
 * data must be set on instantiation. An invalid pointer may cause
 * damage, as no getter method checks data validity. Anyway, a NULL
 * check can be performed by making use of the getWholeData method.
 * @ingroup tmtcpackets
 */
#ifndef TCPACKETBASE_H_
#define TCPACKETBASE_H_

#include <tmtcpacket/SpacePacketBase.h>
#include <utility/HasReturnvaluesIF.h>
#include <stddef.h>

/**
 * This struct defines a byte-wise structured PUS TC Data Field Header.
 * Any optional fields in the header must be added or removed here.
 * Currently, the Source Id field is present with one byte.
 * @ingroup tmtcpackets
 */
typedef struct {
	uint8_t version_type_ack;
	uint8_t service_type;
	uint8_t service_subtype;
	uint8_t source_id;
} PUSTcDataFieldHeader;

/**
 * This struct defines the data structure of a PUS Telecommand Packet when
 * accessed via a pointer.
 * @ingroup tmtcpackets
 */
typedef struct {
	CCSDSPrimaryHeader primary;
	PUSTcDataFieldHeader data_field;
	uint8_t app_data;
} TcPacketPointer;

static const int TCPB_ID = 2;
static const ReturnValue_t TC_BUFFER_TOO_SMALL = MAKE_RETURN_CODE(TCPB_ID, 0x0);

static const uint16_t TC_PACKET_MIN_SIZE = (sizeof(CCSDSPrimaryHeader) +
		sizeof(PUSTcDataFieldHeader) + 2);
/**
 * With this constant for the acknowledge field responses on all levels are expected.
 */
static const uint8_t ACK_ALL = 0b1111;
/**
 * With this constant for the acknowledge field a response on acceptance is expected.
 */
static const uint8_t ACK_ACCEPTANCE = 0b0001;
/**
 * With this constant for the acknowledge field a response on start of execution is expected.
 */
static const uint8_t ACK_START = 0b0010;
/**
 * With this constant for the acknowledge field responses on execution steps are expected.
 */
static const uint8_t ACK_STEP = 0b0100;
/**
 * With this constant for the acknowledge field a response on completion is expected.
 */
static const uint8_t ACK_COMPLETION = 0b1000;
/**
 * With this constant for the acknowledge field no responses are expected.
 */
static const uint8_t ACK_NONE = 0b000;

/**
 * Initializes the Tc Packet header.
 * @param data_ptr
 * A pointer to a buffer. The TC packet will be built inside that buffer
 * @param max_size
 * The size of the supplied buffer. This is required for range checks.
 * @param apid APID used.
 * @param sequenceCount Sequence Count in the primary header.
 * @param ack Which acknowledeges are expected from the receiver.
 * @param service	PUS Service
 * @param subservice PUS Subservice
 */
TcPacketPointer* initialize_tc_packet_header(uint8_t* data_ptr,
		const size_t max_size, uint16_t apid, uint16_t sequenceCount,
		uint8_t ack, uint8_t service, uint8_t subservice);

/**
 * Copies the supplied data to the internal TC application data field.
 * @param pData
 * @param dataLen
 */
ReturnValue_t set_app_data(TcPacketPointer* tc_packet, const size_t max_size,
		uint8_t * app_data, uint16_t data_len);

/**
 * Get read-only data pointer
 * @param tc_packet
 * @return
 */
const uint8_t * get_tc_data_const(TcPacketPointer* tc_packet);
size_t get_full_tc_packet_size(TcPacketPointer* tc_packet);

/**
 * With this method, the Error Control Field is updated to match the
 * current content of the packet.
 */
void set_tc_error_control(TcPacketPointer* tc_packet);

/**
 * This command returns the CCSDS Secondary Header Flag.
 * It shall always be zero for PUS Packets. This is the
 * highest bit of the first byte of the Data Field Header.
 * @return	the CCSDS Secondary Header Flag
 */
uint8_t get_secondary_header_flag(TcPacketPointer* tc_packet);
/**
 * This command returns the TC Packet PUS Version Number.
 * The version number of ECSS PUS 2003 is 1.
 * It consists of the second to fourth highest bits of the
 * first byte.
 * @return
 */
uint8_t get_pus_version_number(TcPacketPointer* tc_packet);
/**
 * This is a getter for the packet's Ack field, which are the lowest four
 * bits of the first byte of the Data Field Header.
 *
 * It is packed in a uint8_t variable.
 * @return	The packet's PUS Ack field.
 */
uint8_t get_tc_acknowledge_flags(TcPacketPointer* tc_packet);
/**
 * This is a getter for the packet's PUS Service ID, which is the second
 * byte of the Data Field Header.
 * @return	The packet's PUS Service ID.
 */
uint8_t get_tc_service(TcPacketPointer* tc_packet);
/**
 * This is a getter for the packet's PUS Service Subtype, which is the
 * third byte of the Data Field Header.
 * @return	The packet's PUS Service Subtype.
 */
uint8_t get_tc_subservice(TcPacketPointer* tc_packet);
/**
 * This is a getter for a pointer to the packet's Application data.
 *
 * These are the bytes that follow after the Data Field Header. They form
 * the packet's application data.
 * @return	A pointer to the PUS Application Data.
 */
const uint8_t* get_application_data(TcPacketPointer* tc_packet);
/**
 * This method calculates the size of the PUS Application data field.
 *
 * It takes the information stored in the CCSDS Packet Data Length field
 * and subtracts the Data Field Header size and the CRC size.
 * @return	The size of the PUS Application Data (without Error Control
 * 		field)
 */
uint16_t get_application_data_size(TcPacketPointer* tc_packet);
/**
 * This getter returns the Error Control Field of the packet.
 *
 * The field is placed after any possible Application Data. If no
 * Application Data is present there's still an Error Control field. It is
 * supposed to be a 16bit-CRC.
 * @return	The PUS Error Control
 */
uint16_t get_tc_error_control(TcPacketPointer* tc_packet);

/**
 * Calculate full packet length from application data length.
 * @param appDataLen
 * @return
 */
size_t calculate_full_tc_packet_length(size_t app_data_len);

#endif /* TCPACKETBASE_H_ */
