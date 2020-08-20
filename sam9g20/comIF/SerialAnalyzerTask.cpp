#include "SerialAnalyzerTask.h"
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/globalfunctions/DleEncoder.h>
#include <cstring>

SerialAnalyzerTask::SerialAnalyzerTask(SharedRingBuffer *ringBuffer,
		AnalyzerModes mode):
		 mode(mode), ringBuffer(ringBuffer) {
	if(ringBuffer == nullptr) {
		sif::error << "SerialAnalyzerTask::SerialAnalyzerTask: "
				"Passed ring buffer invalid!" << std::endl;
	}
	analysisVector = std::vector<uint8_t>(ringBuffer->getMaxSize());

}

ReturnValue_t SerialAnalyzerTask::checkForPackets(uint8_t* receptionBuffer,
		size_t maxSize, size_t* packetSize) {
	if(receptionBuffer == nullptr or packetSize == nullptr) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	ringBuffer->lockRingBufferMutex(MutexIF::TimeoutType::WAITING, 10);
	size_t dataToRead = ringBuffer->getAvailableReadData();
	if(dataToRead == 0) {
		ringBuffer->unlockRingBufferMutex();
		return NO_PACKET_FOUND;
	}

	// Todo: instead of copying data over and over if no STX and ETX are found,
	// we should just copy new data to the end of the analysis vector.
	// we have to cache the current position for that.
	ReturnValue_t result = ringBuffer->readData(analysisVector.data(),
			dataToRead);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		ringBuffer->unlockRingBufferMutex();
		return result;
	}
	ringBuffer->unlockRingBufferMutex();

	if(mode == AnalyzerModes::DLE_ENCODING) {
		size_t readSize = 0;
		result = parseForDleEncodedPackets(dataToRead, receptionBuffer,
				maxSize, packetSize, &readSize);
		if(result == HasReturnvaluesIF::RETURN_OK) {
			// Packet found, advance read pointer.
			ringBuffer->deleteData(readSize);
		}
		else if(result == POSSIBLE_PACKET_LOSS) {
			// ETX found which might be a hint for a possibly lost packet
			ringBuffer->deleteData(readSize);
		}
		// If no packets were found,  we don't do anything.
	}
	return result;
}

ReturnValue_t SerialAnalyzerTask::parseForDleEncodedPackets(
		size_t bytesToRead, uint8_t* receptionBuffer,
		size_t maxSize, size_t* packetSize, size_t* readSize) {
	for(size_t vectorIdx = 0; vectorIdx < bytesToRead; vectorIdx ++) {
		if(analysisVector[vectorIdx] == DleEncoder::STX_CHAR) {
			ReturnValue_t result = DleEncoder::decode(
					&analysisVector[vectorIdx],
					bytesToRead - vectorIdx, readSize,
					receptionBuffer, maxSize, packetSize);
			if(result == HasReturnvaluesIF::RETURN_OK) {
				return HasReturnvaluesIF::RETURN_OK;
			}
			else if(result == DleEncoder::DECODING_ERROR) {
				// should not happen
			}
			else {
				return NO_PACKET_FOUND;
			}
		}
		else if(analysisVector[vectorIdx] == DleEncoder::ETX_CHAR) {
			// might be lost packet, so we should advance the read pointer
			*readSize = ++vectorIdx;
			// std::memset(analysisVector.data(), 0, vectorIdx);
			return POSSIBLE_PACKET_LOSS;
		}
	}
	return NO_PACKET_FOUND;
}
