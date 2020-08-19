#include "MissionMessageTypes.h"
#include <fsfw/modes/ModeMessage.h>

void messagetypes::clearMissionMessage(CommandMessageIF* message) {
	Command_t command = message->getCommand();
	switch((command >> 8) & 0xff) {
	default:
		break;
	}
}


