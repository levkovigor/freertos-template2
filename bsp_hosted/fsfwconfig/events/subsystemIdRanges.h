#ifndef FSFWCONFIG_EVENTS_SUBSYSTEMIDRANGES_H_
#define FSFWCONFIG_EVENTS_SUBSYSTEMIDRANGES_H_

#include "fsfw/events/fwSubsystemIdRanges.h"
#include "common/events/commonSubsystemIds.h"

#include <cstdint>


/**
 * These IDs are part of the ID for an event thrown by a subsystem.
 * Numbers 0-80 are reserved for FSFW Subsystem IDs (framework/events/)
 */
namespace SUBSYSTEM_ID {
enum: uint8_t {
	SUBSYSTEM_ID_START = COMMON_SUBSYSTEM_ID_RANGE
};
}

#endif /* FSFWCONFIG_EVENTS_SUBSYSTEMIDRANGES_H_ */
