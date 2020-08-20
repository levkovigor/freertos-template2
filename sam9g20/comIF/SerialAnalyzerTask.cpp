#include "SerialAnalyzerTask.h"
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/globalfunctions/DleEncoder.h>


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
	size_t excessBytes = ringBuffer->getExcessBytes();
	if(excessBytes > 0) {
		ringBuffer->moveExcessBytesToStart();
	}
	size_t dataToRead = ringBuffer->getAvailableReadData();
	if(dataToRead == 0) {
		ringBuffer->unlockRingBufferMutex();
		return NO_PACKET_FOUND;
	}
	ReturnValue_t result = ringBuffer->readData(analysisVector.data(),
			dataToRead);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		ringBuffer->unlockRingBufferMutex();
		return result;
	}
	ringBuffer->unlockRingBufferMutex();

	if(mode == AnalyzerModes::DLE_ENCODING) {
		result = parseForDleEncodedPackets(dataToRead, receptionBuffer,
				maxSize, packetSize);
		if(result == HasReturnvaluesIF::RETURN_OK) {
			// Packet found, advance read pointer.
			ringBuffer->deleteData(*packetSize);
		}
		else if(result == POSSIBLE_PACKET_LOSS) {
			// ETX found which might be a hint for a possibly lost packet
			ringBuffer->deleteData(*packetSize);
		}
		// If no packets were found,  we don't do anything.
	}
	return result;
}

ReturnValue_t SerialAnalyzerTask::parseForDleEncodedPackets(
		size_t bytesToRead, uint8_t* receptionBuffer,
		size_t maxSize, size_t* packetSize) {
	for(size_t vectorIdx = 0; vectorIdx < bytesToRead; vectorIdx ++) {
		if(analysisVector[vectorIdx] == DleEncoder::STX_CHAR) {
			size_t packetFoundSize = 0;
			ReturnValue_t result = DleEncoder::decode(
					&analysisVector[vectorIdx],
					bytesToRead - vectorIdx, &packetFoundSize,
					receptionBuffer, maxSize, packetSize);
			if(result == HasReturnvaluesIF::RETURN_OK) {
				// packet found
				*packetSize = packetFoundSize;
				return HasReturnvaluesIF::RETURN_OK;
			}
			else {
				return NO_PACKET_FOUND;
			}
		}

		if(analysisVector[vectorIdx] == DleEncoder::ETX_CHAR) {
			// might be lost packet, so we should advance the read pointer
			*packetSize = vectorIdx;
			return POSSIBLE_PACKET_LOSS;
		}
	}
	return NO_PACKET_FOUND;
}
