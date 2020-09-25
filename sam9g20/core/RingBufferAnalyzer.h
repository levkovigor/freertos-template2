#ifndef SAM9G20_UTILITY_SERIALANALYZERHELPER_H_
#define SAM9G20_UTILITY_SERIALANALYZERHELPER_H_

#include <fsfw/container/SharedRingBuffer.h>
#include <vector>

enum class AnalyzerModes {
	DLE_ENCODING //!< DLE encoded packets.
};

/**
 * @brief 	Analyzer task which checks a supplied ring buffer for
 * 			encoded packets.
 * @details
 */
class RingBufferAnalyzer {
public:
	static constexpr uint8_t INTERFACE_ID = CLASS_ID::SERIAL_ANALYZER;
	static constexpr ReturnValue_t NO_PACKET_FOUND = MAKE_RETURN_CODE(0x01);
	static constexpr ReturnValue_t POSSIBLE_PACKET_LOSS = MAKE_RETURN_CODE(0x02);

	/**
	 * Initialize the serial analyzer with a supplied shared ring buffer.
	 * @param buffer
	 * @param mode
	 */
	RingBufferAnalyzer(SharedRingBuffer* buffer,
			AnalyzerModes mode = AnalyzerModes::DLE_ENCODING);

	/**
	 * Search for DLE encoded packets
	 * @param buffer If a packet is found, it will be copied into that buffer
	 * @param maxSize Maximum size of the supplied buffer. Should be large
	 * enough to accomodate maximum designated packet size
	 * @param packetSize Found packet size
	 * @return
	 * -@c RETURN_OK if a packet was found. Read pointer is incremented.
	 * -@c NO_PACKET_FOUND if no packet was found.
	 * -@c POSSIBLE_LOST_PACKET if there is a possiblity of a lost packet
	 *     (end marker found without start marker). Read pointer is incremented.
	 */
	ReturnValue_t checkForPackets(uint8_t* buffer,
			size_t maxData, size_t* packetSize);

private:
	AnalyzerModes mode;
	SharedRingBuffer* ringBuffer;
	std::vector<uint8_t> analysisVector;
	bool dataInAnalysisVectorLeft = false;

	size_t currentBytesRead = 0;

	ReturnValue_t readRingBuffer();
	ReturnValue_t handleDleParsing(uint8_t* receptionBuffer, size_t maxSize,
				size_t* packetSize);
	ReturnValue_t parseForDleEncodedPackets(size_t bytesToRead,
			uint8_t* receptionBuffer, size_t maxSize,
			size_t* packetSize, size_t* readSize);
};



#endif /* SAM9G20_UTILITY_SERIALANALYZERHELPER_H_ */
