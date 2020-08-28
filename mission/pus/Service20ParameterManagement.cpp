#include <mission/pus/Service20ParameterManagement.h>
#include <mission/pus/servicepackets/Service20Packets.h>

#include <apid.h>
#include <pusIds.h>
#include <systemObjectList.h>

#include <fsfw/parameters/HasParametersIF.h>
#include <fsfw/parameters/ParameterMessage.h>
#include <fsfw/parameters/ReceivesParameterMessagesIF.h>


Service20ParameterManagement::Service20ParameterManagement(object_id_t object_id) :
	CommandingServiceBase(object_id,apid::SOURCE_OBSW,pus::PUS_SERVICE_20,
			NUM_OF_PARALLEL_COMMANDS,COMMAND_TIMEOUT_SECONDS) {}

Service20ParameterManagement::~Service20ParameterManagement() {}


ReturnValue_t Service20ParameterManagement::isValidSubservice(uint8_t subservice) {
	switch(static_cast<Subservice>(subservice)) {
	case Subservice::PARAMETER_LOAD:
	case Subservice::PARAMETER_DUMP:
		return HasReturnvaluesIF::RETURN_OK;
	default:
		sif::error << "Invalid Subservice for Service 20" << std::endl;
		return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
	}
}


ReturnValue_t Service20ParameterManagement::getMessageQueueAndObject(
		uint8_t subservice, const uint8_t* tcData, size_t tcDataLen,
		MessageQueueId_t* id, object_id_t* objectId) {
	ReturnValue_t result = checkAndAcquireTargetID(objectId,tcData,tcDataLen);
	if(result != RETURN_OK) {
		return result;
	}
	return checkInterfaceAndAcquireMessageQueue(id,objectId);
}


ReturnValue_t Service20ParameterManagement::checkAndAcquireTargetID(
		object_id_t* objectIdToSet, const uint8_t* tcData, size_t tcDataLen) {
	if(SerializeAdapter::deSerialize(objectIdToSet, &tcData,
			&tcDataLen, SerializeIF::Endianness::BIG) != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "Service 20: Target ID not found. ID: " <<
				std::hex << objectIdToSet ;
		return CommandingServiceBase::INVALID_TC;
	}
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service20ParameterManagement::checkInterfaceAndAcquireMessageQueue(
		MessageQueueId_t* messageQueueToSet, object_id_t* objectId) {
	// check ReceivesParameterMessagesIF property of target
	ReceivesParameterMessagesIF* possibleTarget = objectManager->get<ReceivesParameterMessagesIF>(*objectId);
	if(possibleTarget == nullptr){
		sif::error << "Service20ParameterManagement::checkInterfaceAndAcquire"
			<<	"MessageQueue: Can't access object with ID: "
			<< std::hex << objectId << "as ReceivesParameterMessagesIF"
			<< std::dec << std::endl;
		return CommandingServiceBase::INVALID_OBJECT;
	}
	*messageQueueToSet = possibleTarget->getCommandQueue();
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service20ParameterManagement::prepareCommand(CommandMessage* message,
		uint8_t subservice, const uint8_t* tcData, size_t tcDataLen,
		uint32_t* state, object_id_t objectId) {
	switch(static_cast<Subservice>(subservice)){
	case Subservice::PARAMETER_DUMP: {
		return prepareDumpCommand(message, tcData, tcDataLen);
	}
	break;
	case Subservice::PARAMETER_LOAD: {
		return prepareLoadCommand(message, tcData, tcDataLen);
	}
	break;
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t Service20ParameterManagement::prepareDumpCommand(CommandMessage* message,
		const uint8_t* tcData, size_t tcDataLen) {
	//the first part is the objectId, but we have extracted that earlier and only need the parameterId
	tcData += sizeof(object_id_t);
	tcDataLen -= sizeof(object_id_t);
	ParameterId_t parameterId;
	if(SerializeAdapter::deSerialize(&parameterId, &tcData,
			&tcDataLen, SerializeIF::Endianness::BIG) != HasReturnvaluesIF::RETURN_OK) {
		return CommandingServiceBase::INVALID_TC;
	}
	if(tcDataLen != 0) {  //Autodeserialize should have decremented size to 0 by this point
		return CommandingServiceBase::INVALID_TC;
	}

	ParameterMessage::setParameterDumpCommand(message, parameterId);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service20ParameterManagement::prepareLoadCommand(CommandMessage* message,
		const uint8_t* tcData, size_t tcDataLen) {
	uint8_t* storePointer = nullptr;
	store_address_t storeAddress;
	size_t parameterDataLen = tcDataLen - sizeof(object_id_t) -
			sizeof(ParameterId_t);
	ReturnValue_t result = IPCStore->getFreeElement(&storeAddress,
			parameterDataLen,&storePointer);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	ParameterLoadCommand command(storePointer,parameterDataLen);
	result = command.deSerialize(&tcData, &tcDataLen, SerializeIF::Endianness::BIG);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	ParameterMessage::setParameterLoadCommand(message,
			command.getParameterId(), storeAddress);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service20ParameterManagement::handleReply(const CommandMessage* reply,
		Command_t previousCommand, uint32_t* state,
		CommandMessage* optionalNextCommand, object_id_t objectId,
		bool* isStep) {
	Command_t replyId = reply->getCommand();

	switch(replyId) {
	case ParameterMessage::REPLY_PARAMETER_DUMP: {
		ConstAccessorPair parameterData = IPCStore->getData(ParameterMessage::getStoreId(reply));
		if(parameterData.first != HasReturnvaluesIF::RETURN_OK) {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		ParameterId_t parameterId = ParameterMessage::getParameterId(reply);
		ParameterDumpReply parameterReply(objectId, parameterId,
					parameterData.second.data(), parameterData.second.size());
		sendTmPacket(static_cast<uint8_t>(
				Subservice::PARAMETER_DUMP_REPLY), &parameterReply);
		return HasReturnvaluesIF::RETURN_OK;
	}
	default:
		return CommandingServiceBase::INVALID_REPLY;
	}
}
