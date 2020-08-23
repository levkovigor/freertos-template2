//#include <fsfw/datapool/PoolDataSetBase.h>
#include <fsfw/datapoollocal/LocalDataSet.h>
#include <fsfw/datapoollocal/LocalPoolVariable.h>
#include <fsfw/datapoollocal/LocalPoolVector.h>
#include <fsfw/devicehandlers/AcceptsDeviceResponsesIF.h>
#include <test/testdevices/TestDeviceHandler.h>

#if defined(freeRTOS)
#include <fsfw/osal/FreeRTOS/TaskManagement.h>
#endif

TestDevice::TestDevice(object_id_t objectId, object_id_t comIF,
		CookieIF * cookie, bool onImmediately):
		DeviceHandlerBase(objectId,comIF, cookie),
		onImmediately(onImmediately) {
}

TestDevice::~TestDevice() {}

void TestDevice::debugInterface(uint8_t positionTracker,
		object_id_t objectId, uint32_t parameter) {
}


void TestDevice::performOperationHook() {
	//sif::info << "DummyDevice: Alive!" << std::endl;
	if(mode != MODE_ON and onImmediately) {
		setMode(_MODE_START_UP);
		onImmediately = false;
	}

	if(oneShot) {
		//testLocalDataPool();
		oneShot = false;
	}

#if defined(freeRTOS)
    //sif::info << TaskManagement::getTaskStackHighWatermark() << std::endl;
#endif
	//sif::info << "Hook: " << counter ++ << std::endl;
}

void TestDevice::doStartUp() {
	sif::debug << "DummyDevice: Switching On" << std::endl;
	setMode(_MODE_TO_ON);
	return;
}


void TestDevice::doShutDown() {
	sif::debug << "DummyDevice: Switching Off" << std::endl;
	setMode(_MODE_SHUT_DOWN);
	return;
}


ReturnValue_t TestDevice::buildNormalDeviceCommand(DeviceCommandId_t* id) {
	return NOTHING_TO_SEND;
}

ReturnValue_t TestDevice::buildTransitionDeviceCommand(DeviceCommandId_t* id) {
	if(mode == _MODE_TO_ON) {
		sif::debug << "DummyDevice: buildTransitionDeviceCommand() "
				"was called from _MODE_TO_ON mode" << std::endl;
	}
	if(mode == _MODE_TO_NORMAL) {
		sif::debug << "DummyDevice: buildTransitionDeviceCommand() "
				"was called from _MODE_TO_NORMAL mode" << std::endl;
		setMode(MODE_NORMAL);
	}
	if(mode == _MODE_SHUT_DOWN) {
		sif::debug << "DummyDevice: buildTransitionDeviceCommand() "
				"was called from _MODE_SHUT_DOWN mode" << std::endl;
		setMode(MODE_OFF);
	}
	return NOTHING_TO_SEND;
}

void TestDevice::doTransition(Mode_t modeFrom, Submode_t submodeFrom) {
	if(mode == _MODE_TO_NORMAL) {
		sif::debug << "DummyDevice: Custom transition to normal mode" << std::endl;
	}
	else {
		DeviceHandlerBase::doTransition(modeFrom, submodeFrom);
	}
}

ReturnValue_t TestDevice::buildCommandFromCommand(
		DeviceCommandId_t deviceCommand, const uint8_t* commandData,
		size_t commandDataLen) {
	ReturnValue_t result;
	switch(deviceCommand) {
	case TEST_COMMAND_1:
		result = buildTestCommand1(deviceCommand,commandData,commandDataLen);
		break;
	case TEST_COMMAND_2:
		result = buildTestCommand2(deviceCommand,commandData,commandDataLen);
		break;
	case TEST_COMMAND_3:
		result = buildTestCommand3(deviceCommand,commandData,commandDataLen);
		break;
	default:
		result = DeviceHandlerIF::COMMAND_NOT_SUPPORTED;
	}
	return result;
}


ReturnValue_t TestDevice::buildTestCommand1(DeviceCommandId_t deviceCommand,
        const uint8_t* commandData, size_t commandDataLen) {
	sif::debug << "DummyDevice: Building command from TEST_COMMAND_1" << std::endl;
	if(commandDataLen > 255){
		return DeviceHandlerIF::INVALID_NUMBER_OR_LENGTH_OF_PARAMETERS;
	}
	// The command is passed on in the command buffer as it is
	deviceCommand = EndianConverter::convertBigEndian(deviceCommand);
	memcpy(commandBuffer, &deviceCommand, sizeof(deviceCommand));
	memcpy(commandBuffer + 4, commandData, commandDataLen);
	rawPacket = commandBuffer;
	rawPacketLen = sizeof(deviceCommand) + commandDataLen;
	return RETURN_OK;
}

ReturnValue_t TestDevice::buildTestCommand2(DeviceCommandId_t deviceCommand,
        const uint8_t* commandData,
		size_t commandDataLen) {
	if(commandDataLen < 7){
		return DeviceHandlerIF::INVALID_NUMBER_OR_LENGTH_OF_PARAMETERS;
	}
	sif::debug << "DummyDevice: Building command from TEST_COMMAND_2" << std::endl;
	deviceCommand = EndianConverter::convertBigEndian(deviceCommand);
	memcpy(commandBuffer,&deviceCommand,sizeof(deviceCommand));
	uint16_t parameter1 = 0;
	size_t size = commandDataLen;
	ReturnValue_t result = SerializeAdapter::deSerialize(&parameter1,
			&commandData, &size, SerializeIF::Endianness::BIG);
	if(result==HasReturnvaluesIF::RETURN_FAILED){
		return result;
	}
	if(parameter1 != COMMAND_2_PARAM1){
		return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
	}
	uint64_t parameter2 = 0;
	result = SerializeAdapter::deSerialize(&parameter2,
			&commandData, &size, SerializeIF::Endianness::BIG);
	if(parameter2!= COMMAND_2_PARAM2){
		return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
	}

	commandBuffer[4] = (parameter1 & 0xFF00) >> 8;
	commandBuffer[5] = (parameter1 & 0xFF);
	parameter2 = EndianConverter::convertBigEndian(parameter2);
	memcpy(commandBuffer + 6, &parameter2, sizeof(parameter2));
	rawPacket = commandBuffer;
	rawPacketLen = sizeof(deviceCommand) + sizeof(parameter1) +
			sizeof(parameter2);
	return RETURN_OK;
}

ReturnValue_t TestDevice::buildTestCommand3(DeviceCommandId_t deviceCommand,
        const uint8_t* commandData, size_t commandDataLen) {
	sif::debug << "DummyDevice: Building command from TEST_COMMAND_3" << std::endl;
	if(commandDataLen > 255){
		return DeviceHandlerIF::INVALID_NUMBER_OR_LENGTH_OF_PARAMETERS;
	}
	DeviceCommandId_t commandIdToPass;
	commandIdToPass = EndianConverter::convertBigEndian(deviceCommand);
	// The command is passed on in the command buffer as it is
	memcpy(commandBuffer,&commandIdToPass,sizeof(deviceCommand));
	memcpy(commandBuffer+4,commandData,commandDataLen);
	rawPacket = commandBuffer;
	rawPacketLen = sizeof(deviceCommand)+commandDataLen;
	return RETURN_OK;
}

ReturnValue_t TestDevice::buildTestCommand4(DeviceCommandId_t deviceCommand,
        const uint8_t* commandData, size_t commandDataLen) {
	DeviceCommandId_t commandIdToPass;
	commandIdToPass = EndianConverter::convertBigEndian(deviceCommand);
	// The command is passed on in the command buffer as it is
	memcpy(commandBuffer,&commandIdToPass,sizeof(deviceCommand));
	memcpy(commandBuffer+4,commandData,commandDataLen);
	rawPacket = commandBuffer;
	rawPacketLen = sizeof(deviceCommand)+commandDataLen;
	return RETURN_OK;
}

ReturnValue_t TestDevice::getParameter(uint8_t domainId, uint16_t parameterId,
		ParameterWrapper* parameterWrapper, const ParameterWrapper* newValues,
		uint16_t startAtIndex) {
	//TODO: should the domainID also be checked? What does it do?
	//these values have to be initialized before the switch due to the compiler

	switch (parameterId) {
	case 0:
		parameterWrapper->set(testParameter1);
		break;
	case 1:
		parameterWrapper->set(testParameter2);
		break;
	case 2:
		parameterWrapper->set(testParameter3);
		break;
	default:
		return INVALID_MATRIX_ID;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

void TestDevice::fillCommandAndReplyMap() {
	insertInCommandAndReplyMap(TEST_COMMAND_1,5);
	insertInCommandAndReplyMap(TEST_COMMAND_2,5);
	insertInCommandAndReplyMap(TEST_COMMAND_3,5);
}


ReturnValue_t TestDevice::scanForReply(const uint8_t *start, size_t len,
		DeviceCommandId_t *foundId, size_t *foundLen) {
	if(len < sizeof(object_id_t)){
		return DeviceHandlerIF::LENGTH_MISSMATCH;
	}
	size_t size = len;
	ReturnValue_t result = SerializeAdapter::deSerialize(foundId,
			&start, &size, SerializeIF::Endianness::BIG);
	if (result != RETURN_OK) {
		return result;
	}
	switch(*foundId){
	case TEST_COMMAND_1:
		sif::debug << "DummyDevice: Reply for "
				"TEST_COMMAND_1 (666) received! " << std::endl;
		*foundLen = len;
		return RETURN_OK;
	case TEST_COMMAND_2:
		if(len < TEST_COMMAND_2_SIZE){
			return DeviceHandlerIF::LENGTH_MISSMATCH;
		}
		sif::debug << "DummyDevice: Reply for TEST_COMMAND_2 "
				"(C0C0BABE) received!" << std::endl;
		*foundLen = TEST_COMMAND_2_SIZE;
		return RETURN_OK;
	case TEST_COMMAND_3:
		sif::debug << "DummyDevice: Reply for TEST_COMMAND_3 "
				"(BADEAFFE) received" << std::endl;
		*foundLen = len;
		return RETURN_OK;
	default:
		return DeviceHandlerIF::DEVICE_REPLY_INVALID;
	}
}


ReturnValue_t TestDevice::interpretDeviceReply(DeviceCommandId_t id,
		const uint8_t* packet) {
	ReturnValue_t result;
	switch(id){
	// triggers completion message
	case TEST_COMMAND_1:
		result = interpretingReply1();
		break;
	// triggers data reply
	case TEST_COMMAND_2: {
		result = interpretingReply2(id, packet);
		break;
	}
	// triggers additional step + completion
	case TEST_COMMAND_3: {
		result = interpretingReply3(id, packet);
		break;
	}
	default:
		return DeviceHandlerIF::DEVICE_REPLY_INVALID;
	}
	return result;
}

ReturnValue_t TestDevice::interpretingReply1() {
	CommandMessage directReplyMessage;
	sif::debug << "DummyDevice: Setting Completion Reply for TEST_COMMAND_1" << std::endl;
	ActionMessage::setCompletionReply(&directReplyMessage,
			TEST_COMMAND_1, RETURN_OK);
	return RETURN_OK;
}

ReturnValue_t TestDevice::interpretingReply2(DeviceCommandId_t id,
		const uint8_t* packet) {
	CommandMessage directReplyMessage;
	sif::debug << "DummyDevice: Setting Data Reply for TEST_COMMAND_2" << std::endl;
	// Send reply with data
	store_address_t storeAddress;
	ReturnValue_t result = IPCStore->addData(&storeAddress,packet,14);
	if (result != RETURN_OK) {
		sif::error << "DummyDevice: Could not store TEST_COMMAND_2 packet into "
				 "IPCStore" << std::endl;
		return result;
	}

	ActionMessage::setDataReply(&directReplyMessage,id,storeAddress);
	DeviceCommandMap::iterator commandIter = deviceCommandMap.find(id);
	MessageQueueId_t commander = commandIter->second.sendReplyTo;
	ReturnValue_t sendResult = commandQueue->sendMessage(commander,
			&directReplyMessage,false);
	if(sendResult != RETURN_OK) {
		sif::error << "DummyDevice: Could not issue data reply message" << std::endl;
		return result;
	}
	return RETURN_OK;
}

ReturnValue_t TestDevice::interpretingReply3(DeviceCommandId_t id,
		const uint8_t* packet) {
	CommandMessage commandMessage;
	sif::debug << "Dummy Device: Triggering additional step and "
			"completion for TEST_COMMAND_3" << std::endl;
	DeviceCommandMap::iterator commandIter = deviceCommandMap.find(id);
	MessageQueueId_t commander = commandIter->second.sendReplyTo;
	actionHelper.step(1,commander,id);
	actionHelper.finish(commander,id);

	return RETURN_OK;
}


void TestDevice::setNormalDatapoolEntriesInvalid() {
}

uint32_t TestDevice::getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) {
	return 5000;
}

ReturnValue_t TestDevice::initializePoolEntries(
		LocalDataPool& localDataPoolMap) {
	// This will initialize a uint8_t pool entry with a length of one (uint8_t).
	localDataPoolMap.emplace(static_cast<lp_id_t>(PoolIds::TEST_VAR_1),
			new PoolEntry<uint8_t>());
	// This will initialize a pool entry with 3 floats, which are all
	// 0 initialized
	localDataPoolMap.emplace(static_cast<lp_id_t>(PoolIds::TEST_VEC_1),
			new PoolEntry<float>({0, 0, 0}, 3));
	// This will initialize a pool entry with 3 uint32_t but only
	// initialize the first two values to the specified values
	localDataPoolMap.emplace(static_cast<lp_id_t>(PoolIds::TEST_VEC_2),
			new PoolEntry<uint32_t>({2,5,6}, 3));
	return HasReturnvaluesIF::RETURN_OK;
}

//void TestDevice::testLocalDataPool() {
//	lp_bool_t testBool(static_cast<lp_id_t>(PoolIds::TEST_VAR_1), this);
//	lp_vec_t<float,3> testFloatVec(static_cast<lp_id_t>(
//			PoolIds::TEST_VEC_1), this);
//	lp_vec_t<uint32_t,3> testUint32Vec(static_cast<lp_id_t>(
//			PoolIds::TEST_VEC_2), this);
//	LocalDataSet testDataSet(this, 3);
//	testDataSet.registerVariable(&testBool);
//	testDataSet.registerVariable(&testFloatVec);
//	testDataSet.registerVariable(&testUint32Vec);
//	// First, I want to access a variable without a dataset. This should
//	// be possible.
//	auto res = testBool.read();
//	if(res != RETURN_OK) {
//		sif::error.print("NO");
//	}
//	sif::info << "Read test bool: " <<  (int) testBool.value << std::endl;
//	testBool.value = true;
//	res = testBool.commit();
//	if(res != RETURN_OK) {
//		sif::error.print("NO");
//	}
//	res = testBool.read();
//	if(res != RETURN_OK) {
//		sif::error.print("NO");
//	}
//	sif::info << "Read test bool: " <<  (int) testBool.value << std::endl;
//
//	testUint32Vec.read(MutexIF::POLLING);
//	sif::info << std::dec <<testUint32Vec << std::endl;
//}
