#include "MissionMessageTypes.h"
#include "mission/memory/FileSystemMessage.h"

void messagetypes::clearMissionMessage(CommandMessage* message) {
	switch(message->getMessageType()) {
	case(messagetypes::FILE_SYSTEM_MESSAGE): {
		FileSystemMessage::clear(message);
		break;
	}
	default:
		break;
	}
}


