/**
 * \file Service9Packets.h
 *
 * \brief Structure of a Time Management Telecommand
 * It only contains the time encoded as ASCII, CRC, CUC or CDS
 *
 *  \date 29.08.2019
 *  \author R. Mueller
 */

#include <fsfw/serialize/SerialLinkedListAdapter.h>
#ifndef MISSION_PUS_SERVICEPACKETDEFINITIONS_SERVICE9PACKETS_H_
#define MISSION_PUS_SERVICEPACKETDEFINITIONS_SERVICE9PACKETS_H_


/**
 * \brief Subservice 128
 * \ingroup spacepackets
 */
class TimePacket : SerialLinkedListAdapter<SerializeIF> { //!< [EXPORT] : [SUBSERVICE] 128
public:
	TimePacket(const uint8_t * timeBuffer_, uint32_t timeSize_) {
		timeBuffer = timeBuffer_;
		timeSize = timeSize_;
	}
	const uint8_t*  getTime() {
		return timeBuffer;
	}

	uint32_t getTimeSize() const {
		return timeSize;
	}

private:
	TimePacket(const TimePacket &command);
	const uint8_t * timeBuffer;
	uint32_t timeSize; //!< [EXPORT] : [IGNORE]
};

#endif /* MISSION_PUS_SERVICEPACKETDEFINITIONS_SERVICE8PACKETS_H_ */
