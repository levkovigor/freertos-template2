#ifndef SAM9G20_UTILITY_SERIALANALYZERHELPER_H_
#define SAM9G20_UTILITY_SERIALANALYZERHELPER_H_

#include <fsfw/container/SharedRingBuffer.h>
#include <vector>
#include <map>

enum class AnalyzerModes {
	DLE_ENCODING, //!< DLE encoded packets.
	USLP_FRAMES //!< USLP frames with TRUNCATED primary header
};

/**
 * @brief 	Analyzer task which checks a supplied ring buffer for
 * 			encoded packets.
 * @details
 * @author  R. Mueller
 */
class RingBufferAnalyzer {
public:
	static constexpr uint8_t INTERFACE_ID = CLASS_ID::SERIAL_ANALYZER;
	static constexpr ReturnValue_t NO_PACKET_FOUND = MAKE_RETURN_CODE(1);
	static constexpr ReturnValue_t POSSIBLE_PACKET_LOSS = HasReturnvaluesIF::
	        makeReturnCode(INTERFACE_ID, 2);

	/**
	 * Initialize the serial analyzer for DLE encoding with a supplied shared ring buffer.
	 * @param buffer
	 * @param mode
	 */
	RingBufferAnalyzer(SharedRingBuffer* buffer,
			AnalyzerModes mode = AnalyzerModes::DLE_ENCODING);

    /**
     * Initialize the serial analyzer for fixed size USLP frames a supplied shared ring buffer.
     * @param buffer
     * @param virtualChannelFrameSizes Pointer to map with frame sizes for each VC
     * @param mode
     */
	// TODO: Possibly Reference
    RingBufferAnalyzer(SharedRingBuffer* buffer, std::map<uint8_t, size_t>* virtualChannelFrameSizes,
            AnalyzerModes mode = AnalyzerModes::USLP_FRAMES);

	/**
	 * Search for DLE encoded packets or USLP Frames
	 * @param buffer If a packet/frame is found, it will be copied into that buffer
	 * @param maxSize Maximum size of the supplied buffer. Should be large
	 * enough to accomodate maximum designated packet/frame size
	 * @param packetSize Found packet/frame size
	 * @return
	 * -@c RETURN_OK if a packet/frame was found. Read pointer is incremented.
	 * -@c NO_PACKET_FOUND if no packet/frame was found.
	 * -@c POSSIBLE_LOST_PACKET if there is a possiblity of a lost packets/frame
	 *     (end marker found without start marker / header found within frame). Read pointer is incremented.
	 */
	ReturnValue_t checkForPackets(uint8_t* buffer,
			size_t maxData, size_t* packetSize);

private:
	AnalyzerModes mode;
	SharedRingBuffer* ringBuffer;
	std::vector<uint8_t> analysisVector;
	bool dataInAnalysisVectorLeft = false;
	std::array<uint8_t, 3> uslpHeaderMarker;
	// Stores VC Length map necessary for frame copying
	std::map<uint8_t, size_t>* virtualChannelFrameSizes = nullptr;


	size_t currentBytesRead = 0;

	ReturnValue_t readRingBuffer();
	ReturnValue_t handleParsing(uint8_t* receptionBuffer, size_t maxSize,
				size_t* packetSize);
	ReturnValue_t parseForDleEncodedPackets(size_t bytesToRead,
			uint8_t* receptionBuffer, size_t maxSize,
			size_t* packetSize, size_t* readSize);

	ReturnValue_t parseForUslpFrames(size_t bytesToRead,
            uint8_t* receptionBuffer, size_t maxSize,
            size_t* packetSize, size_t* readSize);
};



#endif /* SAM9G20_UTILITY_SERIALANALYZERHELPER_H_ */
