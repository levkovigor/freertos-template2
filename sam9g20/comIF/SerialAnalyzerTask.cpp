#include "SerialAnalyzerTask.h"
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/globalfunctions/DleEncoder.h>
#include <fsfw/ipc/MutexHelper.h>
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
		size_t maxData, size_t* packetSize) {
	if(receptionBuffer == nullptr or packetSize == nullptr) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	ReturnValue_t result = readRingBuffer();
	if(result == NO_PACKET_FOUND) {
		return result;
	}
	else if(result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	if(mode == AnalyzerModes::DLE_ENCODING) {
		result = handleDleParsing(receptionBuffer, maxData, packetSize);
	}
	return result;
}

ReturnValue_t SerialAnalyzerTask::readRingBuffer() {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	MutexHelper lock(ringBuffer->getMutexHandle(),
			MutexIF::TimeoutType::WAITING, 5);
	size_t dataToRead = ringBuffer->getAvailableReadData();
	if(dataToRead == 0) {
		return NO_PACKET_FOUND;
	}

	if(dataToRead > currentBytesRead) {
		currentBytesRead = dataToRead;
		result = ringBuffer->readData(analysisVector.data(),
				currentBytesRead);
		if(result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
	}
	else {
		// no new data.
		return NO_PACKET_FOUND;
	}
	return result;
}

ReturnValue_t SerialAnalyzerTask::handleDleParsing(uint8_t* receptionBuffer,
		size_t maxSize, size_t* packetSize) {
	size_t readSize = 0;
	ReturnValue_t result = parseForDleEncodedPackets(currentBytesRead,
			receptionBuffer, maxSize, packetSize, &readSize);
	if(result == HasReturnvaluesIF::RETURN_OK) {
		// Packet found, advance read pointer.
		currentBytesRead = 0;
		ringBuffer->deleteData(readSize);
	}
	else if(result == POSSIBLE_PACKET_LOSS) {
		// STX or ETX found at wrong placer
		// which might be a hint for a possibly lost packet.
		currentBytesRead = 0;
		ringBuffer->deleteData(readSize);
	}
	// If no packets were found,  we don't do anything.
	return result;
}

ReturnValue_t SerialAnalyzerTask::parseForDleEncodedPackets(
		size_t bytesToRead, uint8_t* receptionBuffer, size_t maxSize,
		size_t* packetSize, size_t* readSize) {
	bool stxFound = false;
	size_t stxIdx = 0;
	for(size_t vectorIdx = 0; vectorIdx < bytesToRead; vectorIdx ++) {
		// handle STX char
		if(analysisVector[vectorIdx] == DleEncoder::STX_CHAR) {
			if(not stxFound) {
				stxFound = true;
				stxIdx = vectorIdx;
			}
			else {
				// might be lost packet, so we should advance the read pointer
				// without skipping the STX
				*readSize = vectorIdx;
				return POSSIBLE_PACKET_LOSS;
			}
		}
		// handle ETX char
		if(analysisVector[vectorIdx] == DleEncoder::ETX_CHAR) {
			if(stxFound) {
				// This is propably a packet, so we decode it.
				ReturnValue_t result = DleEncoder::decode(
						&analysisVector[stxIdx],
						bytesToRead - stxIdx, readSize,
						receptionBuffer, maxSize, packetSize);
				if(result == HasReturnvaluesIF::RETURN_OK) {
					return HasReturnvaluesIF::RETURN_OK;
				}
				else {
					// invalid packet, skip.
					*readSize = ++vectorIdx;
					return POSSIBLE_PACKET_LOSS;
				}
			}
			else {
				// might be lost packet, so we should advance the read pointer
				*readSize = ++vectorIdx;
				return POSSIBLE_PACKET_LOSS;
			}
		}
	}
	return NO_PACKET_FOUND;
}
