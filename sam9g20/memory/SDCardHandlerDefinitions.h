#ifndef SAM9G20_MEMORY_SDCARDHANDLERDEFINITIONS_H_
#define SAM9G20_MEMORY_SDCARDHANDLERDEFINITIONS_H_

#include <returnvalues/classIds.h>
#include <events/subsystemIdRanges.h>

#include <fsfw/events/Event.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <cstdint>

namespace sdchandler {

static constexpr uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::SD_CARD_HANDLER;
static constexpr uint8_t INTERFACE_ID = CLASS_ID::SD_CARD_HANDLER;

static constexpr ReturnValue_t EXECUTION_COMPLETE =
        HasReturnvaluesIF::makeReturnCode(INTERFACE_ID, 0);

static constexpr Event SD_CARD_SWITCHED = MAKE_EVENT(0x00, severity::MEDIUM); //!< It was not possible to open the preferred SD card so the other was used. P1: Active volume
static constexpr Event SD_CARD_ACCESS_FAILED = MAKE_EVENT(0x01, severity::HIGH); //!< Opening failed for both SD cards.
static constexpr Event SEQUENCE_PACKET_MISSING_WRITE_EVENT = MAKE_EVENT(0x02, severity::LOW); //!< P1: Sequence packet missing.
static constexpr Event SEQUENCE_PACKET_MISSING_READ_EVENT = MAKE_EVENT(0x03, severity::LOW); //!< P1: Sequence packet missing.

}

#endif /* SAM9G20_MEMORY_SDCARDHANDLERDEFINITIONS_H_ */
