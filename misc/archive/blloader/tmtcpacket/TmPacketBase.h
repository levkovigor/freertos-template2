/**
 * @brief 	This file contains functions to create and interface
 * 			ECSS PUS Telemetry packet.
 *
 * In addition to @SpacePacketBase, the class provides methods to handle
 * the standardized entries of the PUS TM Packet Data Field Header.
 * It does not contain the packet data itself but a pointer to the
 * data must be set on instantiation. An invalid pointer may cause
 * damage, as no getter method checks data validity. Anyway, a NULL
 * check can be performed by making use of the getWholeData method.
 * @ingroup tmtcpackets
 */
#ifndef TMPACKETBASE_H_
#define TMPACKETBASE_H_

#include <tmtcpacket/SpacePacketBase.h>
//#include <fsfw/timemanager/TimeStamperIF.h>
//#include <fsfw/timemanager/Clock.h>

#define MISSION_TIMESTAMP_SIZE 8

typedef enum tm_packet_returncode {
TM_PACKET_OK = 0,
TM_BUFFER_TOO_SMALL = 1,
} tm_packet_returncode_t;

static const uint16_t DEFAULT_APID = 's';

/**
 * This struct defines a byte-wise structured PUS TM Data Field Header.
 * Any optional fields in the header must be added or removed here.
 * Currently, no Destination field is present, but an eigth-byte representation
 * for a time tag [TBD].
 * @ingroup tmtcpackets
 */
typedef struct {
	uint8_t version_type_ack;
	uint8_t service_type;
	uint8_t service_subtype;
	uint8_t subcounter;
	//	uint8_t destination;
	//	uint8_t time[TimeStamperIF::MISSION_TIMESTAMP_SIZE];
	uint8_t time[MISSION_TIMESTAMP_SIZE];
} PUSTmDataFieldHeader ;

/**
 * This struct defines the data structure of a PUS Telecommand Packet when
 * accessed via a pointer.
 * @ingroup tmtcpackets
 */
typedef struct {
	CCSDSPrimaryHeader primary;
	PUSTmDataFieldHeader data_field;
	uint8_t data;
} TmPacketPointer;

//! Minimum size of a valid PUS Telemetry Packet.
static const uint32_t  TM_PACKET_MIN_SIZE =
		(sizeof(CCSDSPrimaryHeader) + sizeof(PUSTmDataFieldHeader) + 2);
//! Maximum size of a TM Packet in this mission.
static const uint32_t MISSION_TM_PACKET_MAX_SIZE = 2048;
//! First byte of secondary header for PUS-A packets.
static const uint8_t VERSION_NUMBER_BYTE_PUS_A = 0b00010000;

/**
 * Initializes the Tm Packet header.
 * Does set the timestamp (to now), but not the error control field.
 * @param apid APID used.
 * @param service	PUS Service
 * @param subservice PUS Subservice
 * @param packet_subcounter Additional subcounter used.
 * @param sequence_count TM source sequence count
 */
TmPacketPointer* initialize_tm_packet_header(uint8_t* data_ptr,
		const size_t max_size, uint16_t apid, uint8_t service,
		uint8_t subservice, uint8_t packet_subcounter, uint16_t sequence_count);

/**
 * This sets the source data. It copies the provided data to
 * the internal TmPacketPointer source data location.
 * @param sourceData
 * @param sourceSize
 */
tm_packet_returncode_t set_source_data(TmPacketPointer* tm_packet,
        const size_t max_size, uint8_t* source_data, size_t source_size);

/**
 * With this method, the Error Control Field is updated to match the
 * current content of the packet.
 */
void set_tm_error_control(TmPacketPointer* tm_packet);

/**
 * Getter function to get read-only pointer to packet.
 * @param tm_packet
 * @return
 */
const uint8_t* get_tm_data_const(TmPacketPointer* tm_packet);

/**
 * This is a getter for the packet's PUS Service ID, which is the second
 * byte of the Data Field Header.
 * @return	The packet's PUS Service ID.
 */
uint8_t get_tm_service(TmPacketPointer* tm_packet);

/**
 * This is a getter for the packet's PUS Service Subtype, which is the
 * third byte of the Data Field Header.
 * @return	The packet's PUS Service Subtype.
 */
uint8_t get_tm_subservice(TmPacketPointer* tm_packet);

/**
 * This is a getter for a pointer to the packet's Source data.
 *
 * These are the bytes that follow after the Data Field Header. They form
 * the packet's source data.
 * @return	A pointer to the PUS Source Data.
 */
uint8_t* get_source_data(TmPacketPointer* tm_packet);

/**
 * This method calculates the size of the PUS Source data field.
 *
 * It takes the information stored in the CCSDS Packet Data Length field
 * and subtracts the Data Field Header size and the CRC size.
 * @return	The size of the PUS Source Data (without Error Control field)
 */
uint16_t get_source_data_size(TmPacketPointer* tm_packet);

/**
 * In case data was filled manually (almost never the case).
 * @param size Size of source data (without CRC and data filed header!).
 */
void set_source_data_size(TmPacketPointer* tm_packet, uint16_t size);

/**
 * This getter returns the Error Control Field of the packet.
 *
 * The field is placed after any possible Source Data. If no
 * Source Data is present there's still an Error Control field. It is
 * supposed to be a 16bit-CRC.
 * @return	The PUS Error Control
 */
uint16_t get_tm_error_control(TmPacketPointer* tm_packet);

/**
 * Retrieves the full TM packet length from a supplied TM packet pointer.
 * @param tm_packet
 * @return
 */
size_t get_full_tm_packet_length(TmPacketPointer* tm_packet);

/**
 * Calculates the full TM packet length with a supplied source data length
 * @param source_data_len
 * @return
 */
size_t calculate_full_tm_packet_length(size_t source_data_len);

///**
// * Interprets the "time"-field in the secondary header and returns it in timeval format.
// * @return Converted timestamp of packet.
// */
//ReturnValue_t getPacketTime(timeval* timestamp) const;
///**
// * Returns a raw pointer to the beginning of the time field.
// * @return Raw pointer to time field.
// */
//uint8_t* getPacketTimeRaw() const;
//
//uint32_t getTimestampSize() const;
//
///**
// * Checks if a time stamper is available and tries to set it if not.
// * @return Returns false if setting failed.
// */
//bool checkAndSetStamper();

#endif /* TMPACKETBASE_H_ */
