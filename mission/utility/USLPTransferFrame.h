#ifndef USLPTRANSFERFRAME_H_
#define USLPTRANSFERFRAME_H_

#include <stdint.h>
#include <stddef.h>

/**
 * The USLPTransferFrame class simplifies handling of such Frames.
 * It operates on any buffer passed on construction. The data length
 * is determined by the length field in the frame itself.
 * It has a lot of getters for convenient access to the content.
 * @ingroup ccsds_handling
 */
class USLPTransferFrame {
protected:
	/**
	 * The struct that defines the Frame's Primary Header.
	 */
	struct USLPTransferFramePrimaryHeader {
		uint8_t tfvnAndScid;	//!< Highest byte with TFVN and SCID
		uint8_t scid_m;	//!< Byte with middle of SCID
		uint8_t scidAndsourceflagAndVcid;	//!< Byte with rest of SCID, source flag, and start of VCID.
		uint8_t vcidAndMapidAndtruncatedflag;	//!< Byte with rest of VCID, MAP ID and End of Frame Primary Header flag
	};
	/**
	 * The struct defining the whole Transfer Frame.
	 */
	struct uslp_transfer_frame {
		USLPTransferFramePrimaryHeader header;	//!< The header struct.
		uint8_t dataField;				//!< The data field of the Transfer Frame.
	};
	uslp_transfer_frame* frame;			//!< Pointer to a buffer where a Frame is placed.
public:
	static const uint8_t FRAME_CRC_SIZE = 2;	//!< Constant for the CRC size.
	/**
	 * Empty Constructor that sets the data pointer to NULL.
	 */
	USLPTransferFrame();
	/**
	 * The data pointer passed in this Constructor is casted to the #uslp_transfer_frame struct.
	 * @param setData The data on which the class shall operate.
	 */
	USLPTransferFrame(uint8_t* setData);
	/**
	 * Getter.
	 * @return The Version number.
	 */
	uint8_t getVersionNumber();
	/**
	 * Getter.
	 * @return The Spacecraft Identifier.
	 */
	uint16_t getSpacecraftId();
	/**
	 * Getter.
	 * @return	If the source or destination flag is set or not.
	 */
	bool sourceFlagSet();
	/**
	 * Getter.
	 * @return The Virtual Channel Identifier.
	 */
	uint8_t getVirtualChannelId();
	/**
	 * Getter.
	 * @return The Multiplexer Access Point Identifier from the Segment Header byte.
	 */
	uint8_t getMAPId();
	/**
	 * Getter.
	 * @return	If the End of Frame Primary Header flag is set or not.
	 */
	bool truncatedFlagSet();


	/* Possibly useful/*

//	/**
//	 * Getter.
//	 * @return The Frame length as stored in the Header.
//	 */
//	uint16_t getFrameLength();
//	/**
//	 * Getter.
//	 * @return The length of pure data (without CRC), assuming that a Segment Header is present.
//	 */
//	uint16_t getDataLength();
//	/**
//	 * Getter.
//	 * @return The length of pure data (without CRC), assuming that no Segment Header is present (for BC Frames).
//	 */
//	uint16_t getFullDataLength();
//	/**
//	 * Getter.
//	 * @return A pointer to the date field AFTER a Segment Header.
//	 */
//	uint8_t* getDataField();
//	/**
//	 * Getter.
//	 * @return A pointer to the first byte in the Data Field (ignoring potential Segment Headers, for BC Frames).
//	 */
//	uint8_t* getFullDataField();
//	/**
//	 * Getter.
//	 * @return A pointer to the beginning of the Frame.
//	 */
//	uint8_t* getFullFrame();
//	/**
//	 * Getter
//	 * @return The total size of the Frame, which is the size stated in the Header + 1.
//	 */
//	uint16_t getFullSize();
//	/**
//	 * Getter.
//	 * @return Size of the #TcTransferFramePrimaryHeader.
//	 */
//	uint16_t getHeaderSize();
//	/**
//	 * Debug method to print the whole Frame to screen.
//	 */
//	void print();

};

#endif /* USLPTRANSFERFRAME_H_ */
