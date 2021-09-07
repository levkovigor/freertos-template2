#ifndef USLPTRANSFERFRAME_H_
#define USLPTRANSFERFRAME_H_

#include <stdint.h>
#include <stddef.h>

/**
 * This class simplifies handling of USLP Transfer Frames.
 * It works on a buffer supplied at construction, make sure it is large
 * enough for the frame.
 * It only includes the truncated primary frame header version,
 * so data zone size has to be supplied at construction.
 * @author L. Rajer
 */
class USLPTransferFrame {
protected:
    /**
     * The struct that defines the Frame's Primary Header.
     */
    struct USLPTransferFramePrimaryHeader {
        uint8_t tfvnAndScid;	//!< Highest byte with TFVN and SCID
        uint8_t scid_m;	//!< Byte with middle of SCID
        uint8_t scidAndsourceflagAndVcid;//!< Byte with rest of SCID, source flag, and start of VCID.
        uint8_t vcidAndMapidAndtruncatedflag;//!< Byte with rest of VCID, MAP ID and End of Frame Primary Header flag
    };
    /**
     * The struct that defines the Frame's Data Zone Header.
     */
    struct USLPTransferFrameDataFieldHeader {
        uint8_t rulesAndprotocolid;	//!< Highest byte with TFDZ Construction Rules and USLP Protocol Identifier
        // This has to be done in two bytes, a uint16_t does not work here
        uint8_t firstHeaderPointer_h;	//!< First Bytes with First Header Pointer
        uint8_t firstHeaderPointer_l;  //!< Second Bytes with First Header Pointer
    };
    /**
     * The struct defining the whole Transfer Frame.
     */
    struct uslp_transfer_frame {
        USLPTransferFramePrimaryHeader primaryHeader;	//!< The primary header struct.
        USLPTransferFrameDataFieldHeader dataFieldHeader; //!< The data field header struct.
        uint8_t dataZone;				//!< The data field of the Transfer Frame.
    };
    uslp_transfer_frame *frame;			//!< Pointer to a buffer where a Frame is placed.
    uint16_t dataZoneSize;				//!< Transfer Frame Data Zone Size
public:
    static const uint8_t FRAME_CRC_SIZE = 2;	//!< Constant for the CRC size.
    static const uint8_t FRAME_OVERHEAD = FRAME_CRC_SIZE + 7;//!< Constant for total frame overhead
    /**
     * Empty Constructor that sets the data pointer to NULL and data zone size to 0
     */
    USLPTransferFrame();
    /**
     * The data pointer passed in this Constructor is casted to the #uslp_transfer_frame struct.
     * @param setData The data on which the class shall operate.
     * @param dataZoneSize Size of TFDZ, necessary because of Truncated Primary Header
     */
    USLPTransferFrame(uint8_t *setData, uint16_t dataZoneSize);

    /** @brief Changes the buffer where the frame points to
     */
    void setFrameLocation(uint8_t *setData, uint16_t dataZoneSize);
    /**
     * Getter.
     * @return The Version number.
     */
    uint8_t getVersionNumber();
    /**
     * Setter.
     * @param versionNumber Transfer Frame Version Number (4 bits)
     */
    void setVersionNumber(uint8_t versionNumber);
    /**
     * Getter.
     * @return The Spacecraft Identifier.
     */
    uint16_t getSpacecraftId();
    /**
     * Setter.
     * @param spacecraftID Spacecraft ID, (16 bits)
     */
    void setSpacecraftId(uint16_t spacecraftID);
    /**
     * Getter.
     * @return	If the source or destination flag is set or not. (0/False: Origin client, 1/True: Origin OBC)
     */
    bool sourceFlagSet();
    /**
     * Setter.
     * @param isSet	Sets source or destination flag (0/False: Origin client, 1/True: Origin OBC)
     */
    void setSourceFlag(bool isSet);
    /**
     * Getter.
     * @return The Virtual Channel Identifier.
     */
    uint8_t getVirtualChannelId();
    /**
     * Setter.
     * @param The Virtual Channel Identifier (6 bits)
     */
    void setVirtualChannelId(uint8_t virtualChannelId);
    /**
     * Getter.
     * @return The Multiplexer Access Point Identifier.
     */
    uint8_t getMapId();
    /**
     * Setter.
     * @param mapId The Multiplexer Access Point Identifier (4 bits)
     */
    void setMapId(uint8_t mapId);
    /**
     * Getter.
     * @return	If the End of Frame Primary Header flag is set or not.
     */
    bool truncatedFlagSet();
    /**
     * Setter.
     * @param isSet	Sets truncated flag (0/False: Full Header, 1/True: Truncated)
     * @warning Behavior for non truncated headers not implemented, so only use this way
     */
    void setTruncatedFlag(bool isSet);
    /**
     * Getter.
     * @return	The Transfer Frame Data Zone Construction Rules.
     */
    uint8_t getTFDZConstructionRules();
    /**
     * Setter.
     * @param constructionRules Transfer Frame Data Zone Construction Rules (3 bits)
     */
    void setTFDZConstructionRules(uint8_t constructionRules);
    /**
     * Getter.
     * @return	The Transfer Frame Data Zone Protocol Identifier (e.g. PUS Packets).
     */
    uint8_t getProtocolIdentifier();
    /**
     * Setter.
     * @param protocolId Transfer Frame Data Zone Protocol Identifier (5 bits)
     */
    void setProtocolIdentifier(uint8_t protocolId);
    /**
     * Getter.
     * @return	Octet offset to first header. (0 if header is first octet, if no header 0xFFFF)
     */
    uint16_t getFirstHeaderOffset();
    /**
     * Setter.
     * @param offset Transfer Frame Data Zone First header octet offset (0 if header is first octet, if no header 0xFFFF)
     */
    void setFirstHeaderOffset(uint16_t offset);
    /**
     * Getter.
     * @return	Pointer to first header, nullpointer if no header present in frame
     */
    uint8_t* getFirstHeader();
    /**
     * Getter.
     * @return The total frame length with CRC.
     */
    uint16_t getFullFrameSize();
    /**
     * Getter.
     * @return The length of the data zone (without CRC).
     */
    uint16_t getDataZoneSize();
    /**
     * Getter.
     * @return A pointer to the first byte in the Data Zone.
     */
    uint8_t* getDataZone();
    /**
     * Getter.
     * @return A pointer to the beginning of the Frame.
     */
    uint8_t* getFullFrame();

//	/**
//	 * Debug method to print the whole Frame to screen.
//	 */
//	void print();

};

#endif /* USLPTRANSFERFRAME_H_ */
