#ifndef FSFWCONFIG_IPC_MISSIONMESSAGETYPES_H_
#define FSFWCONFIG_IPC_MISSIONMESSAGETYPES_H_

#include <fsfw/ipc/CommandMessage.h>
#include <fsfw/ipc/FwMessageTypes.h>

/**
 * Custom command messages are specified here.
 * Most messages needed to use FSFW are already located in
 * <fsfw/ipc/FwMessageTypes.h>
 * @param message Generic Command Message
 */
namespace messagetypes {
enum CustomMessageTypes {
	MISSION_MESSAGE_TYPE_START = FW_MESSAGES_COUNT
};

void clearMissionMessage(CommandMessage* message);
}

#endif /* FSFWCONFIG_IPC_MISSIONMESSAGETYPES_H_ */
