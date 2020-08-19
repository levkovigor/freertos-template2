#include <mission/pus/CService201HealthCommanding.h>
#include <apid.h>
#include <pusIds.h>
#include <systemObjectList.h>

#include <fsfw/health/HasHealthIF.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/health/HealthMessage.h>
#include <mission/pus/servicepackets/Service201Packets.h>

CService201HealthCommanding::CService201HealthCommanding(object_id_t objectId_):
	CommandingServiceBase(objectId_,apid::SOURCE_OBSW, pus::PUS_SERVICE_201,
			NUMBER_OF_PARALLEL_COMMANDS,COMMAND_TIMEOUT_SECONDS) {
}

CService201HealthCommanding::~CService201HealthCommanding() {
}

ReturnValue_t CService201HealthCommanding::isValidSubservice(uint8_t subservice) {
	switch(subservice) {
	case(Subservice::COMMAND_SET_HEALTH):
		return RETURN_OK;
	default:
		sif::error << "Invalid Subservice" << std::endl;
		return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
	}
}


ReturnValue_t CService201HealthCommanding::getMessageQueueAndObject(uint8_t subservice,
		const uint8_t *tcData, size_t tcDataLen,
		MessageQueueId_t *id, object_id_t *objectId)
{
	ReturnValue_t result = checkAndAcquireTargetID(objectId,tcData,tcDataLen);
	if (result != RETURN_OK) {
		return result;
	}
	result = checkInterfaceAndAcquireMessageQueue(id,objectId);
	return result;
}

ReturnValue_t CService201HealthCommanding::checkAndAcquireTargetID
		(object_id_t* objectIdToSet, const uint8_t* tcData, size_t tcDataLen)
{
	if (SerializeAdapter::deSerialize(objectIdToSet, &tcData,&tcDataLen,
	        SerializeIF::Endianness::BIG) != HasReturnvaluesIF::RETURN_OK) {
		return CommandingServiceBase::INVALID_TC;
	} else {
		return HasReturnvaluesIF::RETURN_OK;
	}
}

ReturnValue_t CService201HealthCommanding::checkInterfaceAndAcquireMessageQueue
		(MessageQueueId_t* MessageQueueToSet, object_id_t* objectId) {
	HasHealthIF * possibleTarget = objectManager->get<HasHealthIF>(*objectId);
	if(possibleTarget != NULL) {
		*MessageQueueToSet = possibleTarget->getCommandQueue();
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return CommandingServiceBase::INVALID_OBJECT;
	}
}

ReturnValue_t CService201HealthCommanding::prepareCommand
		(CommandMessage* message, uint8_t subservice, const uint8_t *tcData,
		size_t tcDataLen, uint32_t *state, object_id_t objectId) {
	HealthCommand healthCommand;
	ReturnValue_t result = healthCommand.deSerialize(&tcData, &tcDataLen,
	        SerializeIF::Endianness::BIG);
	if (result != RETURN_OK) {
		return result;
	} else {
		HealthMessage::setHealthMessage(dynamic_cast<CommandMessage*>(message),
				HealthMessage::HEALTH_SET, healthCommand.getHealth());
		return result;
	}
}

ReturnValue_t CService201HealthCommanding::handleReply
		(const CommandMessage* reply, Command_t previousCommand,
		uint32_t *state, CommandMessage* optionalNextCommand,
		object_id_t objectId, bool *isStep) {
	Command_t replyId = reply->getCommand();
	if (replyId == HealthMessage::REPLY_HEALTH_SET) {
		prepareHealthSetReply(reply);
	}
	return RETURN_OK;
}

void CService201HealthCommanding::prepareHealthSetReply(
        const CommandMessage* reply) {
	prepareHealthSetReply(reply);
	uint8_t health = static_cast<uint8_t>(HealthMessage::getHealth(reply));
	uint8_t oldHealth = static_cast<uint8_t>(HealthMessage::getOldHealth(reply));
	HealthSetReply healthSetReply(health, oldHealth);
	sendTmPacket(Subservice::REPLY_HEALTH_SET,&healthSetReply);
}

