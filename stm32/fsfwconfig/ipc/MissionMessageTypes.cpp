#include <fsfw/modes/ModeMessage.h>
#include <stm32/fsfwconfig/ipc/MissionMessageTypes.h>

void messagetypes::clearMissionMessage(CommandMessageIF* message) {
	Command_t command = message->getCommand();
	switch((command >> 8) & 0xff) {
	default:
		break;
	}
}


