#ifndef SPACEPACKETBASE_H_
#define SPACEPACKETBASE_H_

#include <tmtcpacket/ccsds_header.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @defgroup tmtcpackets Space Packets
 * This is the group, where all classes associated with Telecommand and
 * Telemetry packets belong to.
 * The class hierarchy resembles the dependency between the different standards
 * applied, namely the CCSDS Space Packet standard and the ECCSS Packet
 * Utilization Standard. Most field and structure names are taken from these
 * standards.
 */

/**
 * This struct defines the data structure of a Space Packet when accessed
 * via a pointer.
 * @ingroup tmtcpackets
 */
typedef struct {
	CCSDSPrimaryHeader header;
	uint8_t packet_data;
} SpacePacketPointer;


static const uint16_t LIMIT_APID = 2048; //2^11
static const uint16_t LIMIT_SEQUENCE_COUNT = 16384; // 2^14
static const uint16_t APID_IDLE_PACKET = 0x7FF;
static const uint8_t TELECOMMAND_PACKET = 1;
static const uint8_t TELEMETRY_PACKET = 0;
/**
 * This definition defines the CRC size in byte.
 */
static const uint8_t CRC_SIZE = 2;
/**
 * This is the minimum size of a SpacePacket.
 */
static const uint16_t MINIMUM_SIZE = sizeof(CCSDSPrimaryHeader) + CRC_SIZE;

void init_space_packet_header(SpacePacketPointer* spacePacket,
		bool isTelecommand, bool hasSecondaryHeader, uint16_t apid,
		uint16_t sequenceCount);

//CCSDS Methods:
/**
 * Getter for the packet version number field.
 * @return Returns the highest three bit of the packet in one byte.
 */
uint8_t get_packet_version_number(SpacePacketPointer* packet);
/**
 * This method checks the type field in the header.
 * This bit specifies, if the command is interpreted as Telecommand of
 * as Telemetry. For a Telecommand, the bit is set.
 * @return Returns true if the bit is set and false if not.
 */
bool is_telecommand(SpacePacketPointer* packet);


/**
 * The CCSDS header provides a secondary header flag (the fifth-highest bit),
 *  which is checked with this method.
 * @return	Returns true if the bit is set and false if not.
 */
bool has_secondary_header(SpacePacketPointer* packet);
/**
 * Returns the complete first two bytes of the packet, which together form
 * the CCSDS packet id.
 * @return	The CCSDS packet id.
 */
uint16_t get_packet_id(SpacePacketPointer* packet);
/**
 * Returns the APID of a packet, which are the lowest 11 bit of the packet
 * id.
 * @return The CCSDS APID.
 */
uint16_t get_apid(SpacePacketPointer* packet);
/**
 * Sets the APID of a packet, which are the lowest 11 bit of the packet
 * id.
 * @param 	The APID to set. The highest five bits of the parameter are
 * 			ignored.
 */
void set_apid(SpacePacketPointer* spacePacket, uint16_t setAPID );
/**
 * Returns the CCSDS packet sequence control field, which are the third and
 * the fourth byte of the CCSDS primary header.
 * @return The CCSDS packet sequence control field.
 */
uint16_t getPacketSequenceControl(SpacePacketPointer* packet);
/**
 * Returns the SequenceFlags, which are the highest two bit of the packet
 * sequence control field.
 * @return	The CCSDS sequence flags.
 */
uint8_t get_sequence_flags(SpacePacketPointer* packet);
/**
 * Returns the packet sequence count, which are the lowest 14 bit of the
 * packet sequence control field.
 * @return The CCSDS sequence count.
 */
uint16_t get_packet_sequence_count(SpacePacketPointer* packet);
/**
 * Sets the packet sequence count, which are the lowest 14 bit of the
 * packet sequence control field.
 * setCount is modulo-divided by \c LIMIT_SEQUENCE_COUNT to avoid overflows.
 * @param setCount	The value to set the count to.
 */
void set_packet_sequence_count(SpacePacketPointer* spacePacket, uint16_t setCount);
/**
 * Returns the packet data length, which is the fifth and sixth byte of the
 * CCSDS Primary Header. The packet data length is the size of every kind
 * of data \b after the CCSDS Primary Header \b -1.
 * @return The CCSDS packet data length.
 */
//uint16_t is sufficient, because this is limit in CCSDS standard
uint16_t get_packet_data_length(SpacePacketPointer* packet);
/**
 * Sets the packet data length, which is the fifth and sixth byte of the
 * CCSDS Primary Header.
 * @param setLength The value of the length to set. It must fit the true
 * 					CCSDS packet data length . The packet data length is
 * 					the size of every kind of data \b after the CCSDS
 * 					Primary Header \b -1.
 */
void set_packet_data_length(SpacePacketPointer* packet, uint16_t set_length );

// Helper methods:
uint8_t* get_packet_data();

/**
 * This method returns the full raw packet size.
 * @return	The full size of the packet in bytes.
 */
size_t get_full_space_packet_size(SpacePacketPointer* packet);

uint32_t get_apid_and_sequence_count(SpacePacketPointer* packet);

uint8_t* get_whole_data(SpacePacketPointer* packet);

#endif /* SPACEPACKETBASE_H_ */
