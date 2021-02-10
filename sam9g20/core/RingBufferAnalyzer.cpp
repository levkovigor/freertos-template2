#include "RingBufferAnalyzer.h"

#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/globalfunctions/DleEncoder.h>
#include <fsfw/ipc/MutexHelper.h>
#include <fsfwconfig/OBSWConfig.h>

#include <cstring>
#include <sam9g20/comIF/RS485DeviceComIF.h>
#include "../../mission/utility/uslpDataLinkLayer/USLPTransferFrame.h"

RingBufferAnalyzer::RingBufferAnalyzer(SharedRingBuffer *ringBuffer, AnalyzerModes mode) :
        mode(mode), ringBuffer(ringBuffer) {
    if (ringBuffer == nullptr) {
        sif::error << "SerialAnalyzerTask::SerialAnalyzerTask: "
                "Passed ring buffer invalid!" << std::endl;
    } else if (mode != AnalyzerModes::DLE_ENCODING) {
        sif::error << "SerialAnalyzerTask::SerialAnalyzerTask: "
                "Wrong constructor for DLE mode!" << std::endl;
    }
    analysisVector = std::vector<uint8_t>(ringBuffer->getMaxSize());

}

RingBufferAnalyzer::RingBufferAnalyzer(SharedRingBuffer *ringBuffer,
        std::map<uint8_t, size_t> *virtualChannelFrameSizes, AnalyzerModes mode) :
        mode(mode), ringBuffer(ringBuffer), virtualChannelFrameSizes(virtualChannelFrameSizes) {

    if (ringBuffer == nullptr) {
        sif::error << "SerialAnalyzerTask::SerialAnalyzerTask: "
                "Passed ring buffer invalid!" << std::endl;
    } else if (mode != AnalyzerModes::USLP_FRAMES) {
        sif::error << "SerialAnalyzerTask::SerialAnalyzerTask: "
                "Wrong constructor for USLP mode!" << std::endl;
    } else if (virtualChannelFrameSizes == nullptr || virtualChannelFrameSizes->empty()) {
        sif::error << "SerialAnalyzerTask::SerialAnalyzerTask: "
                "No VCID lengths provided!" << std::endl;
    }

    analysisVector = std::vector<uint8_t>(ringBuffer->getMaxSize());

    // The first 20 bits of the USLP header are constant and used instead of a sync marker
    uslpHeaderMarker[0] = (config::RS485_USLP_TFVN << 4) + (config::RS485_USLP_SCID >> 12);
    uslpHeaderMarker[1] = (config::RS485_USLP_SCID & 0x0FF0) >> 4;
    uslpHeaderMarker[2] = (config::RS485_USLP_SCID & 0x000f) << 4;

}

ReturnValue_t RingBufferAnalyzer::checkForPackets(uint8_t *receptionBuffer, size_t maxData,
        size_t *packetSize) {
    if (receptionBuffer == nullptr or packetSize == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    ReturnValue_t result = readRingBuffer();
    if (result == NO_PACKET_FOUND) {
        return result;
    } else if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    result = handleParsing(receptionBuffer, maxData, packetSize);

    return result;
}

ReturnValue_t RingBufferAnalyzer::readRingBuffer() {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    MutexHelper lock(ringBuffer->getMutexHandle(), MutexIF::TimeoutType::WAITING,
            config::RS232_MUTEX_TIMEOUT);
    size_t dataToRead = ringBuffer->getAvailableReadData();
    if (dataToRead == 0) {
        return NO_PACKET_FOUND;
    }

    if (dataToRead > currentBytesRead) {
        currentBytesRead = dataToRead;
        result = ringBuffer->readData(analysisVector.data(), currentBytesRead);
        if (result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
    } else {
        // no new data.
        return NO_PACKET_FOUND;
    }
    return result;
}

ReturnValue_t RingBufferAnalyzer::handleParsing(uint8_t *receptionBuffer, size_t maxSize,
        size_t *packetSize) {
    size_t readSize = 0;
    ReturnValue_t result = NO_PACKET_FOUND;
    if (mode == AnalyzerModes::DLE_ENCODING) {
        result = parseForDleEncodedPackets(currentBytesRead, receptionBuffer, maxSize,
                packetSize, &readSize);
    } else if (mode == AnalyzerModes::USLP_FRAMES) {
        result = parseForUslpFrames(currentBytesRead, receptionBuffer, maxSize, packetSize,
                &readSize);
    }
    if (result == HasReturnvaluesIF::RETURN_OK) {
        // Packet found, advance read pointer.
        currentBytesRead = 0;
        ringBuffer->deleteData(readSize);
    } else if (result == POSSIBLE_PACKET_LOSS) {
        // Markers found at wrong place
        // which might be a hint for a possibly lost packet.
        currentBytesRead = 0;
        ringBuffer->deleteData(readSize);
    }
    // If no packets were found,  we don't do anything.
    return result;
}

ReturnValue_t RingBufferAnalyzer::parseForDleEncodedPackets(size_t bytesToRead,
        uint8_t *receptionBuffer, size_t maxSize, size_t *packetSize, size_t *readSize) {
    bool stxFound = false;
    size_t stxIdx = 0;
    for (size_t vectorIdx = 0; vectorIdx < bytesToRead; vectorIdx++) {
        // handle STX char
        if (analysisVector[vectorIdx] == DleEncoder::STX_CHAR) {
            if (not stxFound) {
                stxFound = true;
                stxIdx = vectorIdx;
            } else {
                // might be lost packet, so we should advance the read pointer
                // without skipping the STX
                *readSize = vectorIdx;
                return POSSIBLE_PACKET_LOSS;
            }
        }
        // handle ETX char
        if (analysisVector[vectorIdx] == DleEncoder::ETX_CHAR) {
            if (stxFound) {
                // This is probably a packet, so we decode it.

                ReturnValue_t result = DleEncoder::decode(&analysisVector[stxIdx],
                        bytesToRead - stxIdx, readSize, receptionBuffer, maxSize, packetSize);
                if (result == HasReturnvaluesIF::RETURN_OK) {
                    return HasReturnvaluesIF::RETURN_OK;
                } else {
                    // invalid packet, skip.
                    *readSize = ++vectorIdx;
                    return POSSIBLE_PACKET_LOSS;
                }
            } else {
                // might be lost packet, so we should advance the read pointer
                *readSize = ++vectorIdx;
                return POSSIBLE_PACKET_LOSS;
            }
        }
    }
    return NO_PACKET_FOUND;
}

ReturnValue_t RingBufferAnalyzer::parseForUslpFrames(size_t bytesToRead, uint8_t *receptionBuffer,
        size_t maxSize, size_t *packetSize, size_t *readSize) {
    std::map<uint8_t, size_t>::iterator iter = virtualChannelFrameSizes->end();
    //TODO: Check against maxSize and bytesToRead
    //TODO: Increment readPointer
    // This only works for byte aligned data
    for (size_t vectorIdx = 0; vectorIdx <= bytesToRead - config::RS485_MIN_SERIAL_FRAME_SIZE;
            vectorIdx++) {

        // Check first 20 bits of packet for frame
        if (analysisVector[vectorIdx] == uslpHeaderMarker[0]
                && analysisVector[vectorIdx + 1] == uslpHeaderMarker[1]
                && (analysisVector[vectorIdx + 2] & 0xF0) == uslpHeaderMarker[2]) {
            // Get VCID
            // We don't need to analyze the whole frame here, so we just get the relevant bits
            uint8_t vcid = ((analysisVector[vectorIdx + 2] & 0b00000111) << 3)
                    + ((analysisVector[vectorIdx + 3] & 0b11100000) >> 5);

            iter = virtualChannelFrameSizes->find(vcid);

            if (iter == virtualChannelFrameSizes->end()) {
                // If we don't know the VCID, we either read at the wrong place, or we received an
                // unknown VCID, both are not good, so we just skip this section by advancing the
                // read pointer
                *readSize = ++vectorIdx;
                return POSSIBLE_PACKET_LOSS;
            }
            else{
                *packetSize = iter->second;
                // If we would decode the USLP Frame here this would be more relevant, here it is
                // just to be compatible with the DLE encoding part
                *readSize = iter->second;
                (void) std::memcpy(receptionBuffer, &analysisVector[vectorIdx], *packetSize);
                return HasReturnvaluesIF::RETURN_OK;
            }

        }

    }
    return NO_PACKET_FOUND;
}
