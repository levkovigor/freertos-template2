#include <mission/pus/Service3Housekeeping.h>
#include <mission/pus/servicepackets/Service3Packets.h>

#include <systemObjectList.h>
#include <apid.h>
#include <pusIds.h>

#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>


Service3Housekeeping::Service3Housekeeping(object_id_t objectId, uint16_t apid,
			uint8_t serviceId):
		CommandingServiceBase(objectId, apid, serviceId,
		NUM_OF_PARALLEL_COMMANDS, COMMAND_TIMEOUT_SECONDS) {}

Service3Housekeeping::~Service3Housekeeping() {}

ReturnValue_t Service3Housekeeping::isValidSubservice(uint8_t subservice) {
	switch(static_cast<Subservice>(subservice)) {
	case Subservice::ADD_HK_REPORT_STRUCTURE:
	case Subservice::ADD_DIAGNOSTICS_REPORT_STRUCTURE:
	case Subservice::DELETE_HK_REPORT_STRUCTURE:
	case Subservice::DELETE_DIAGNOSTICS_REPORT_STRUCTURE:
	case Subservice::ENABLE_PERIODIC_HK_REPORT_GENERATION:
	case Subservice::DISABLE_PERIODIC_HK_REPORT_GENERATION:
	case Subservice::ENABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION:
	case Subservice::DISABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION:
	case Subservice::REPORT_HK_REPORT_STRUCTURES:
	case Subservice::REPORT_DIAGNOSTICS_REPORT_STRUCTURES :
	case Subservice::HK_DEFINITIONS_REPORT:
	case Subservice::DIAGNOSTICS_DEFINITION_REPORT:
	case Subservice::HK_REPORT:
	case Subservice::DIAGNOSTICS_REPORT:
	case Subservice::GENERATE_ONE_PARAMETER_REPORT:
	case Subservice::GENERATE_ONE_DIAGNOSTICS_REPORT:
	case Subservice::APPEND_PARAMETERS_TO_PARAMETER_REPORT_STRUCTURE:
	case Subservice::APPEND_PARAMETERS_TO_DIAGNOSTICS_REPORT_STRUCTURE:
	case Subservice::MODIFY_PARAMETER_REPORT_COLLECTION_INTERVAL:
	case Subservice::MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL:
		return HasReturnvaluesIF::RETURN_OK;
	default:
		return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
	}
}

ReturnValue_t Service3Housekeeping::getMessageQueueAndObject(uint8_t subservice,
		const uint8_t *tcData, size_t tcDataLen,
		MessageQueueId_t *id, object_id_t *objectId) {
	return HasReturnvaluesIF::RETURN_OK;
	ReturnValue_t result = checkAndAcquireTargetID(objectId,tcData,tcDataLen);
	if(result != RETURN_OK) {
		return result;
	}
	return checkInterfaceAndAcquireMessageQueue(id,objectId);
}

ReturnValue_t Service3Housekeeping::checkAndAcquireTargetID(
		object_id_t* objectIdToSet, const uint8_t* tcData, size_t tcDataLen) {
	if(SerializeAdapter::deSerialize(objectIdToSet, &tcData, &tcDataLen,
	        SerializeIF::Endianness::BIG) != HasReturnvaluesIF::RETURN_OK) {
		return CommandingServiceBase::INVALID_TC;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service3Housekeeping::checkInterfaceAndAcquireMessageQueue(
		MessageQueueId_t* messageQueueToSet, object_id_t* objectId) {
	// check OwnsLocalDataPoolIF property of target
	HasLocalDataPoolIF* possibleTarget =
			objectManager->get<HasLocalDataPoolIF>(*objectId);
	if(possibleTarget == nullptr){
		return CommandingServiceBase::INVALID_OBJECT;
	}
	*messageQueueToSet = possibleTarget->getCommandQueue();
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service3Housekeeping::prepareCommand(CommandMessage* message,
		uint8_t subservice, const uint8_t *tcData, size_t tcDataLen,
		uint32_t *state, object_id_t objectId) {
	switch(static_cast<Subservice>(subservice)) {
	case Subservice::ADD_HK_REPORT_STRUCTURE:
	case Subservice::ADD_DIAGNOSTICS_REPORT_STRUCTURE:
	case Subservice::DELETE_HK_REPORT_STRUCTURE:
	case Subservice::DELETE_DIAGNOSTICS_REPORT_STRUCTURE:
	case Subservice::ENABLE_PERIODIC_HK_REPORT_GENERATION:
	case Subservice::DISABLE_PERIODIC_HK_REPORT_GENERATION:
	case Subservice::ENABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION:
	case Subservice::DISABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION:
	case Subservice::REPORT_HK_REPORT_STRUCTURES:
	case Subservice::REPORT_DIAGNOSTICS_REPORT_STRUCTURES :
	case Subservice::GENERATE_ONE_PARAMETER_REPORT:
	case Subservice::GENERATE_ONE_DIAGNOSTICS_REPORT:
	case Subservice::APPEND_PARAMETERS_TO_PARAMETER_REPORT_STRUCTURE:
	case Subservice::APPEND_PARAMETERS_TO_DIAGNOSTICS_REPORT_STRUCTURE:
	case Subservice::MODIFY_PARAMETER_REPORT_COLLECTION_INTERVAL:
	case Subservice::MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL:
		break;
	case Subservice::HK_DEFINITIONS_REPORT:
	case Subservice::DIAGNOSTICS_DEFINITION_REPORT:
	case Subservice::HK_REPORT:
	case Subservice::DIAGNOSTICS_REPORT:
		// Those are telemetry packets.
		return CommandingServiceBase::INVALID_TC;
	default:
		// should never happen.
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service3Housekeeping::handleReply(const CommandMessage* reply,
		Command_t previousCommand, uint32_t *state,
		CommandMessage* optionalNextCommand, object_id_t objectId,
		bool *isStep) {
	switch(reply->getCommand()) {
	case(HousekeepingMessage::HK_REPORT):
		return generateHkReport(reply);
	default:
		sif::error << "Service3Housekeeping::handleReply: Invalid reply!"
				<< std::endl;
		return CommandingServiceBase::INVALID_REPLY;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

void Service3Housekeeping::handleUnrequestedReply(
        CommandMessage* reply) {
    switch(reply->getCommand()) {
    case(HousekeepingMessage::HK_REPORT): {
        generateHkReport(reply);
        break;
    }
    default: {
        sif::error << "Service3Housekeeping::handleUnrequestedReply: "
                << "Invalid reply!" << std::endl;
    }
    }
}

MessageQueueId_t Service3Housekeeping::getHkQueue() const {
	return commandQueue->getId();
}

ReturnValue_t Service3Housekeeping::generateHkReport(
		const CommandMessage* hkMessage) {
	store_address_t storeId;
	sid_t sid = HousekeepingMessage::getHkReportMessage(hkMessage, &storeId);
	auto resultPair = IPCStore->getData(storeId);
	if(resultPair.first != HasReturnvaluesIF::RETURN_OK) {
		return resultPair.first;
	}
	HkPacket hkPacket(sid, resultPair.second.data(), resultPair.second.size());

	auto result = sendTmPacket(static_cast<uint8_t>(Subservice::HK_REPORT),
			hkPacket.hkData, hkPacket.hkSize,
			reinterpret_cast<uint8_t*>(&hkPacket.sid), sizeof(sid));
	return result;

}

