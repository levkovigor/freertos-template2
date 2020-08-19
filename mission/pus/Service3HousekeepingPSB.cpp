#include <mission/pus/Service3HousekeepingPSB.h>

#include <apid.h>
#include <pusIds.h>
#include <sid.h>

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/serialize/SerializeAdapter.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/tmtcservices/AcceptsTelemetryIF.h>
#include <fsfw/serialize/EndianConverter.h>
#include <fsfw/datapool/PoolRawAccessHelper.h>
#include <fsfw/osal/FreeRTOS/TaskManagement.h>

Service3HousekeepingPSB::Service3HousekeepingPSB(object_id_t object_id,
		uint16_t apid, uint8_t serviceId):
	PusServiceBase(object_id, apid, serviceId),
	hkPool(object_id, SIZE_OF_HK_ELEMENTS, NUMBER_OF_HK_ELEMENTS),
	intervalCounter(1), sidHkMap(sid::MAX_NUMBER_OF_HK_DEFINITIONS),
	sidDiagnosticsMap(sid::MAX_NUMBER_OF_DIAGNOSTICS_DEFINITIONS),
	errCountMap(MAX_ERROR_MAP_ENTRIES) {
}

Service3HousekeepingPSB::~Service3HousekeepingPSB() {
}

ReturnValue_t Service3HousekeepingPSB::performService() {
	//sif::info << TaskManagement::getTaskStackHighWatermark() << std::endl;
	// All HK definitions are checked once per cycle
	generateAllPackets();
	if(cycleCounter == CHECK_ERROR_MAP_TRIGGER) {
		checkErrorMap();
		cycleCounter = 1;
	}
	else {
		cycleCounter ++;
	}
	return RETURN_OK;
}

void Service3HousekeepingPSB::generateAllPackets() {
	// HK definitons are checked every second, diagnostics every 0.2 seconds
	if (intervalCounter == sid::HK_INTERV_FACTOR) {
		generateHkPacketsFrom(&sidHkMap);
		intervalCounter = 1;
	}
	else {
		intervalCounter ++;
	}
	generateHkPacketsFrom(&sidDiagnosticsMap);
}


void Service3HousekeepingPSB::checkErrorMap() {
	ErrorCounterMapIterator iter;
	for(iter = errCountMap.begin();iter != errCountMap.end(); iter++) {
		if(iter.second().errorCounter >= ERROR_MAP_DISABLE_TRIGGER) {
			disableErrorMapEntry(&iter);
		}
	}
}

void Service3HousekeepingPSB::generateHkPacketsFrom(
		SidToStoreIdMap *sidToPoolIdMap) {
	SidToStoreIdIterator sidIterator;
	HkPacketDefinition hkDefinition;
	isDiagnostics = true;
	if(sidToPoolIdMap == &sidHkMap) {
		isDiagnostics = false;
	}
	for(sidIterator=sidToPoolIdMap->begin();
			sidIterator!=sidToPoolIdMap->end();sidIterator ++)
	{
		if(sidIterator == sidToPoolIdMap->end()) {
			return;
		}
		store_address_t storeId = sidIterator.second();
		currentSid = sidIterator.first();
		ReturnValue_t result = retrieveHkPacketDefinition(storeId, &hkDefinition);
		uint32_t testParam = 0;
		std::memcpy(&testParam, hkDefinition.getParameters(), sizeof(testParam));
		if(result == RETURN_OK) {
			checkHkPacketGeneration(storeId, &hkDefinition);
		}
		else {
			updateErrorMap(currentSid, result);
		}
	}
}

// check whether hk packer generation is actually necessary first
void Service3HousekeepingPSB::checkHkPacketGeneration(store_address_t storeId,
		HkPacketDefinition * hkDefinition) {
	ReturnValue_t result = RETURN_FAILED;
	uint32_t currentCycleCounter = hkDefinition->getCycleCounter();
	//info << (int)currentCycleCounter << std::endl;
	uint32_t collectionIntervalInTicks =
			sid::INTERVAL_SECONDS_TO_INTERVAL(isDiagnostics,
			hkDefinition->getCollectionInterval());
	if(hkDefinition->reportingEnabled()
			and currentCycleCounter >= collectionIntervalInTicks)
	{
		//sif::info << "Service 3: Collection Interval" <<
		//		hkDefinition->getCollectionInterval() << std::endl;
		handleHkPacketGeneration(currentSid, hkDefinition);
	}
	else if(hkDefinition->reportingEnabled()) {
		hkDefinition->setCycleCounter(++currentCycleCounter);
	}

	result = updateHkPoolEntry(storeId, hkDefinition);
	if(result != RETURN_OK) {
		updateErrorMap(currentSid, result);
	}
}

void Service3HousekeepingPSB::handleHkPacketGeneration(uint32_t sid,
		HkPacketDefinition * hkDefinition) {
	ReturnValue_t result = RETURN_FAILED;
	uint8_t subservice = 0;
	if(isDiagnostics) {
		subservice = Subservice::DIAGNOSTICS_REPORT;
	}
	else {
		subservice = Subservice::HK_REPORT;
	}

	result = generateHkPacket(sid, hkDefinition, subservice);

	if(result == RETURN_OK) {
		hkDefinition->setCycleCounter(1);
		//debug << "Service 3: HK packet with SID "
		//	    << std::hex << sid << " generated !" << std::endl;
	}
	else {
		sif::error << "Service 3: Error generating HK packet" << std::endl;
		updateErrorMap(currentSid, result);
	}
}


/* Periodic Error Handling */

void Service3HousekeepingPSB::updateErrorMap(sid32_t currentSID,
		ReturnValue_t returnCode) {
	if(not commandedFromGround) {
		ErrorMapValue * currentError = NULL;
		ReturnValue_t result = errCountMap.find(currentSID,&currentError);
		if(result == RETURN_OK) {
			//info << "Current error counter: "
			//     << (int)currentError->errorCounter << std::endl;
			currentError->errorCounter++;
			currentError->returnCode = returnCode;
		}
		else if(result == ErrorCounterMap::KEY_DOES_NOT_EXIST) {
			initializeErrorMapEntry(currentError, returnCode);
		}
	}
}

void Service3HousekeepingPSB::initializeErrorMapEntry(ErrorMapValue * currentError,
		ReturnValue_t returnCode) {
	//info << "Initializing Error Map Entry !" << std::endl;
	currentError->errorCounter = 1;
	currentError->isDiagnostics = isDiagnostics;
	currentError->returnCode = returnCode;
	ReturnValue_t result = errCountMap.insert(
			ErrorCounterMapEntry(currentSid,*currentError));
	if(result == ErrorCounterMap::MAP_FULL) {
		sif::debug << "Service 3: Error map is full ! Consider increasing the map size" << std::endl;
	}
	else if(result != RETURN_OK) {
		sif::error << "Service 3: Configuration Error !" << std::endl;
	}
}

void Service3HousekeepingPSB::disableErrorMapEntry(ErrorCounterMapIterator * iter) {
	sid32_t currentSID = iter->first();
	ReturnValue_t result = RETURN_FAILED;
	if(iter->second().isDiagnostics) {
		sif::info << "Disabling Diagnostics entry with SID: "
			 << std::hex << (int)currentSID << std::dec << std::endl;
		result = disableDiagnosticsReporting(currentSID);
		if(result != RETURN_OK) {
			sif::error << "Service 3: Error map configuration error, "
					 "could not disable Diagnostics store entry !" << std::endl;
		}
	}
	else {
		// info << "Disabling HK entry with SID: " << std::hex
		//	 << (int)currentSID << std::dec << std::endl;
		result = disableHkReporting(currentSID);
		if(result != RETURN_OK) {
			sif::error << "Service 3: Error map configuration error, "
					 "could not disable HK store entry!" << std::endl;
		}
	}
	triggerEvent(POOL_ENTRY_DISABLED,currentSID, iter->second().returnCode);
	errCountMap.erase(iter);
}

ReturnValue_t Service3HousekeepingPSB::disableHkReporting(uint32_t sid) {
	isDiagnostics = false;
	ReturnValue_t result = switchReportGeneration(sid, &sidHkMap, false);
	return result;
}

ReturnValue_t Service3HousekeepingPSB::disableDiagnosticsReporting(uint32_t sid) {
	isDiagnostics = true;
	ReturnValue_t result = switchReportGeneration(sid, &sidDiagnosticsMap, false);
	return result;
}


/* Subservice Handling */

ReturnValue_t Service3HousekeepingPSB::handleRequest(uint8_t subservice) {
	// check whether subservice is valid and whether TC
	// is intended for diagnostics or HK
	ReturnValue_t result = checkSubservice(subservice);
	if (result != RETURN_OK) {
		sif::error << "Service 3: Invalid Subservice !" << std::endl;
		return result;
	}
	commandedFromGround = true;
	switch(subservice) {
	case(ADD_HK_REPORT_STRUCTURE):
	case(ADD_DIAGNOSTICS_REPORT_STRUCTURE):
		result = handleAddingReportStructure();
		break;
	case(DELETE_HK_REPORT_STRUCTURE):
	case(DELETE_DIAGNOSTICS_REPORT_STRUCTURE):
		result = handleDeletingReportStructure();
		break;
	case(ENABLE_PERIODIC_HK_REPORT_GENERATION):
	case(ENABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION):
		result = switchPeriodicReportGeneration(true);
		break;
	case(DISABLE_PERIODIC_HK_REPORT_GENERATION):
	case(DISABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION):
		result = switchPeriodicReportGeneration(false);
		break;
	case(REPORT_HK_REPORT_STRUCTURES):
	case(REPORT_DIAGNOSTICS_REPORT_STRUCTURES):
		result = handleStructureReporting();
		break;
	case(GENERATE_ONE_PARAMETER_REPORT):
	case(GENERATE_ONE_DIAGNOSTICS_REPORT):
		result = generateOneParameterReport();
		break;
	case(APPEND_PARAMETERS_TO_PARAMETER_REPORT_STRUCTURE):
	case(APPEND_PARAMETERS_TO_DIAGNOSTICS_REPORT_STRUCTURE):
		result = appendParametersToReport();
		break;
	case(MODIFY_PARAMETER_REPORT_COLLECTION_INTERVAL):
	case(MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL):
		result = modifyCollectionInterval();
		break;
	default:
		return RETURN_FAILED;
	};
	commandedFromGround = false;
	return result;
}

ReturnValue_t Service3HousekeepingPSB::checkSubservice(uint8_t subservice) {
	isDiagnostics = false;
	switch(currentPacket.getSubService()) {
	case(ADD_HK_REPORT_STRUCTURE):
	case(DELETE_HK_REPORT_STRUCTURE):
	case(ENABLE_PERIODIC_HK_REPORT_GENERATION):
	case(DISABLE_PERIODIC_HK_REPORT_GENERATION):
	case(REPORT_HK_REPORT_STRUCTURES):
	case(GENERATE_ONE_PARAMETER_REPORT):
	case(APPEND_PARAMETERS_TO_PARAMETER_REPORT_STRUCTURE):
	case(MODIFY_PARAMETER_REPORT_COLLECTION_INTERVAL):
		return RETURN_OK;
	case(ADD_DIAGNOSTICS_REPORT_STRUCTURE):
	case(DELETE_DIAGNOSTICS_REPORT_STRUCTURE):
	case(ENABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION):
	case(DISABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION):
	case(REPORT_DIAGNOSTICS_REPORT_STRUCTURES):
	case(GENERATE_ONE_DIAGNOSTICS_REPORT):
	case(APPEND_PARAMETERS_TO_DIAGNOSTICS_REPORT_STRUCTURE):
	case(MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL):
		isDiagnostics = true;
		return RETURN_OK;
	default:
		return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
	}
}

ReturnValue_t Service3HousekeepingPSB::handleAddingReportStructure() {
	ReturnValue_t result;
	HkPacketDefinition hkReportToAdd;
	const uint8_t * data = currentPacket.getApplicationData();
	size_t size = currentPacket.getApplicationDataSize();
	result = hkReportToAdd.deSerialize(&data, &size,
	        SerializeIF::Endianness::BIG, commandedFromGround);
	if(result != RETURN_OK) {
		sif::debug << "Service 3: Error deserializing sent HK structure" << std::endl;
		return result;
	}
	result = addPacketDefinitionToStore(&hkReportToAdd, isDiagnostics);
	if(result != RETURN_OK) {
		sif::debug << "Service 3: Error adding sent HK structure to HK pool" << std::endl;
	}
	return result;
}

ReturnValue_t Service3HousekeepingPSB::handleDeletingReportStructure() {
	ReturnValue_t result;
	uint32_t sid;
	if((result = retrieveSID(&sid)) != RETURN_OK) {
		return result;
	}
	if(isDiagnostics) {
		result = deleteReportStructure(sid, &sidDiagnosticsMap);
	}
	else {
		result = deleteReportStructure(sid, &sidHkMap);
	}
	return result;
}

ReturnValue_t Service3HousekeepingPSB::switchPeriodicReportGeneration(
		bool enableGeneration) {
	ReturnValue_t result = RETURN_FAILED;
	uint32_t sid;
	if((result = retrieveSID(&sid)) != RETURN_OK) {
		return result;
	}
	if(isDiagnostics) {
		result = switchReportGeneration(sid, &sidDiagnosticsMap, enableGeneration);
	}
	else {
		result = switchReportGeneration(sid, &sidHkMap, enableGeneration);
	}
	return result;
}

ReturnValue_t Service3HousekeepingPSB::switchReportGeneration(uint32_t sid,
		SidToStoreIdMap * sidMap, bool enableGeneration) {
	store_address_t storeId;
	HkPacketDefinition hkPacket;
	ReturnValue_t result =
			retrieveHkPacketDefinitionFromSid(sid, &storeId, &hkPacket);
	if(result == RETURN_OK) {
		hkPacket.setReportingStatus(enableGeneration);
		result = updateHkPoolEntry(storeId, &hkPacket);
	}
	return result;
}

ReturnValue_t Service3HousekeepingPSB::handleStructureReporting() {
	ReturnValue_t result;
	uint32_t sid;
	HkPacketDefinition hkPacket;
	uint8_t subservice;
	if(isDiagnostics) {
		subservice = Subservice::DIAGNOSTICS_DEFINITION_REPORT;
	}
	else {
		subservice = Subservice::HK_DEFINITIONS_REPORT;
	}
	if((result = retrieveSID(&sid)) != RETURN_OK) {
		return result;
	}

	result = retrieveHkPacketDefinitionFromSid(sid, &hkPacket);
	if (result != RETURN_OK) {
		return result;
	}

	// Set up required format for PUS standard. SID required !
	hkPacket.setStructureReportLinks(sid);
	TmPacketStored tmPacket(apid::SOURCE_OBSW, pus::PUS_SERVICE_3, subservice,
			packetSubCounter++, &hkPacket);
	result = tmPacket.sendPacket(
			requestQueue->getDefaultDestination(), requestQueue->getId());
	return result;
}

ReturnValue_t Service3HousekeepingPSB::generateOneParameterReport()  {
	uint32_t sid;
	HkPacketDefinition hkPacket;
	ReturnValue_t result;
	uint8_t subservice;
	if(isDiagnostics) {
		subservice = Subservice::DIAGNOSTICS_REPORT;
	}
	else {
		subservice = Subservice::HK_REPORT;
	}
	if((result = retrieveSID(&sid)) != RETURN_OK) {
		return result;
	}

	result = retrieveHkPacketDefinitionFromSid(sid,&hkPacket);
	if (result != RETURN_OK) {
		return result;
	}

	result = generateHkPacket(sid, &hkPacket, subservice);
	return result;
}

ReturnValue_t Service3HousekeepingPSB::modifyCollectionInterval() {
	hkModifyCollectionIntervalCommand hkModCollIntvCommand;
	const uint8_t * data = currentPacket.getApplicationData();
	size_t size = currentPacket.getApplicationDataSize();
	hkModCollIntvCommand.deSerialize(&data,
			&size, SerializeIF::Endianness::BIG);
	uint32_t sid = hkModCollIntvCommand.getSID();
	store_address_t storeId;
	HkPacketDefinition hkPacket;

	ReturnValue_t result =
			retrieveHkPacketDefinitionFromSid(sid, &storeId, &hkPacket);
	if(result != RETURN_OK) {
		return result;
	}

	hkPacket.setCollectionInterval(hkModCollIntvCommand.getInterval_seconds());
	result = updateHkPoolEntry(storeId,&hkPacket);
	return result;
}

ReturnValue_t Service3HousekeepingPSB::appendParametersToReport() {
	hkAppendParametersCommand hkAppendParamsCommand;
	const uint8_t * data = currentPacket.getApplicationData();
	size_t size = currentPacket.getApplicationDataSize();
	hkAppendParamsCommand.deSerialize(&data,&size,
	        SerializeIF::Endianness::BIG);
	uint32_t sid = hkAppendParamsCommand.getSID();
	store_address_t storeId;
	HkPacketDefinition oldHkPacket;
	SidToStoreIdMap * targetMap;
	if(isDiagnostics) {
		targetMap = &sidDiagnosticsMap;
	}
	else {
		targetMap = &sidHkMap;
	}
	ReturnValue_t result = retrieveHkPacketDefinitionFromSid(sid, &oldHkPacket);
	if(result != RETURN_OK) {
		return result;
	}

	uint32_t updatedData [sid::MAX_LARGE_HK_BUFFER_SIZE/4];
	uint8_t updatedParamLen;
	prepareNewData(updatedData, &updatedParamLen,
			&oldHkPacket, &hkAppendParamsCommand);
	result = deleteReportStructure(sid,targetMap);
	return result;
	// update with cycle count
	HkPacketDefinition newHkPacket;
	newHkPacket.initialize(sid, oldHkPacket.getCollectionInterval(),
			updatedParamLen,reinterpret_cast<uint32_t *>(updatedData),
			oldHkPacket.reportingEnabled());
	result = addPacketDefinitionToStore(&newHkPacket, isDiagnostics);
	return result;
}

void Service3HousekeepingPSB::prepareNewData(uint32_t * updatedData,
		uint8_t * updatedNumberOfParams, HkPacketDefinition * oldHkPacket,
		hkAppendParametersCommand * newParams) {
	uint32_t * oldParams = oldHkPacket->getParameters();
	uint8_t oldParamLen = oldHkPacket->getNumberOfParameters();
	uint8_t sizeOfOldParams = oldParamLen * 4;

	const uint32_t * dataToAppend = newParams->getParameters();
	uint8_t numberOfNewParams = newParams->getNumberOfParameters();
	uint8_t sizeOfDataToAppend = numberOfNewParams * 4;

	memcpy(updatedData, oldParams, oldParamLen);
	memcpy(updatedData + sizeOfOldParams, dataToAppend, sizeOfDataToAppend);
	*updatedNumberOfParams = oldParamLen + numberOfNewParams;
}

ReturnValue_t Service3HousekeepingPSB::retrieveSID(uint32_t * sid) {
	const uint8_t * data = currentPacket.getApplicationData();
	size_t size = currentPacket.getApplicationDataSize();
	ReturnValue_t result = SerializeAdapter::deSerialize(sid, &data, &size,
	        SerializeIF::Endianness::BIG);
	if(result != RETURN_OK) {
		sif::error << "Service 3: Error Serializing SID" << std::endl;
		return RETURN_FAILED;
	} else {
		return RETURN_OK;
	}
}

ReturnValue_t Service3HousekeepingPSB::retrieveStoreID(uint32_t sid,
		SidToStoreIdMap * sidMap, store_address_t * storeId) {
	SidToStoreIdIterator sidIterator = sidMap->find(sid);
	if(sidIterator != sidMap->end()) {
		*storeId = sidIterator.second();
		return RETURN_OK;
	}
	else {
		triggerEvent(SID_NOT_FOUND,sid,currentPacket.getSubService());
		return RETURN_FAILED;
	}
}


/* Insertion Functions */

ReturnValue_t Service3HousekeepingPSB::addPacketDefinitionToStore
			(HkPacketDefinition * hkPacketDefinition, bool isDiagnostics) {
	sid32_t sid = hkPacketDefinition->getSID();
	if(hkPacketDefinition->getPacketFormat() !=
			HkPacketDefinition::packetFormat::HK_POOL)
	{
		hkPacketDefinition->prepareGroundPacketForHkPool();
	}
	SidToStoreIdMap * sidMap;
	if(isDiagnostics) {
		sidMap = &sidDiagnosticsMap;
	}
	else {
		sidMap = &sidHkMap;
	}
	ReturnValue_t result = addPacketDefinitionToStore(sid,
			sidMap, hkPacketDefinition);
	return result;
}

ReturnValue_t Service3HousekeepingPSB::addPacketDefinitionToStore(sid32_t sid,
		SidToStoreIdMap * sidMap, HkPacketDefinition * hkPacketDefinition) {
	// check whether SID already exists in SID map or whether SID map is full
	ReturnValue_t result = checkSidMap(sid, sidMap);
	if(result != RETURN_OK) {
		return result;
	}

	store_address_t storeId = 0;
	uint8_t * destinationPointer = nullptr;
	result = hkPool.getFreeElement(&storeId,
			hkPacketDefinition->getSerializedSize(), &destinationPointer);
	if(result != RETURN_OK) {
		return result;
	}
	size_t size = 0;
	result = hkPacketDefinition->serialize(&destinationPointer,
			&size, MAX_HK_DATA_SIZE, SerializeIF::Endianness::MACHINE);
	if(result != RETURN_OK) {
		return result;
	}
	// Add (SID,StoreID) pair to map so the storeID can be retrieved later
	result = handleSidMapInsertion(sid, sidMap , storeId);
	if(result != RETURN_OK) {
		sif::warning << "Service 3: Adding packet definition failed" << std::endl;
		result = hkPool.deleteData(storeId);
		if(result != RETURN_OK) {
			sif::debug << "Service 3: Config error, could not delete data "
					 "that should have been added" << std::endl;
		}
	}
	return result;
}


ReturnValue_t Service3HousekeepingPSB::checkSidMap(uint32_t sid,
		SidToStoreIdMap * sidMap) {
	if(sidMap->full()) {
		sif::debug << "Service 3: Config error, Sid Map full !" << std::endl;
		errorParameter1 = sid;
		if(sidMap == &sidHkMap) {
			errorParameter2 = sid::MAX_NUMBER_OF_HK_DEFINITIONS;
		}
		else {
			errorParameter2 = sid::MAX_NUMBER_OF_DIAGNOSTICS_DEFINITIONS;
		}
		return SID_STORE_MAP_FULL;
	}
	ReturnValue_t result = sidMap->exists(sid);
	if(result == SidToStoreIdMap::KEY_DOES_NOT_EXIST) {
		result = RETURN_OK;
	}
	else {
		sif::info << "Service 3: That key already exists" << std::endl;
		result = SID_ALREADY_EXISTS;
		errorParameter1 = sid;
	}
	return result;
}

ReturnValue_t Service3HousekeepingPSB::handleSidMapInsertion(uint32_t sid,
		SidToStoreIdMap * sidMap, store_address_t storeId) {
	ReturnValue_t result = sidMap->insert(SidToStoreIdMapEntry(sid, storeId));
	switch(result) {
		case(RETURN_OK):
			break;
		case(SidToStoreIdMap::KEY_ALREADY_EXISTS):
			sif::error << "Service 3: Error adding SID to Storage Map,"
					 " key already exists" << std::endl;
			break;
		case(SidToStoreIdMap::MAP_FULL):
			sif::error << "Service 3: Error adding SID to "
					 "Storage Map, map is full" << std::endl;
			break;
		default:
			sif::debug << "Service 3: Adding to SID Map, "
					 "config error, invalid ReturnValue" << std::endl;
	};
	return result;
}


/* Retrieval functins */

ReturnValue_t Service3HousekeepingPSB::retrieveHkPacketDefinitionFromSid(sid32_t sid,
		HkPacketDefinition * hkPacket) {
	store_address_t storeId = 0;
	return retrieveHkPacketDefinitionFromSid(sid, &storeId, hkPacket);
}

ReturnValue_t Service3HousekeepingPSB::retrieveHkPacketDefinitionFromSid(sid32_t sid,
		store_address_t * storeId, HkPacketDefinition * hkPacket) {
	SidToStoreIdMap * targetMap = NULL;
	if(isDiagnostics) {
		targetMap = &sidDiagnosticsMap;
	}
	else {
		targetMap = &sidHkMap;
	}
	ReturnValue_t result = retrieveStoreID(sid,targetMap,storeId);
	if(result != RETURN_OK) {
		return result;
	}
	result = retrieveHkPacketDefinition(*storeId, hkPacket);
	return result;
}

ReturnValue_t Service3HousekeepingPSB::retrieveHkPacketDefinition(
		store_address_t storeId, HkPacketDefinition * hkPacket) {
	const uint8_t * recvBuffer = const_cast<const uint8_t *>(hkBuffer);
	size_t size = 0;
	ReturnValue_t result = retrieveHkDefinitionBufferFromPool(
			&recvBuffer, storeId, &size);
	if(result == RETURN_OK) {
		result = transformDefinitionBufferIntoObject(hkPacket,
			&recvBuffer, &size);
	}
	return result;
}

ReturnValue_t Service3HousekeepingPSB::retrieveHkDefinitionBufferFromPool
		(const uint8_t ** recvBuffer, store_address_t storeId, size_t * size) {
	ReturnValue_t result = hkPool.getData(storeId,recvBuffer,size);
	if(result != RETURN_OK) {
		sif::error << "Service 3: Could not retrieve HK data"
				 " with store ID" << std::endl;
		return result;
	}
	return RETURN_OK;
}

ReturnValue_t Service3HousekeepingPSB::transformDefinitionBufferIntoObject(
		HkPacketDefinition * hkDefinition, const uint8_t ** recvBuffer,
		size_t * size) {
	ReturnValue_t result = RETURN_FAILED;
	result = hkDefinition->deSerialize(recvBuffer, size,
	        SerializeIF::Endianness::MACHINE);

	if(*size != 0) {
		sif::error << "Service 3: Size missmatch when "
				 "retrieving HK definition" << std::endl;
		result = SIZE_MISSMATCH;
	}
	if(result != RETURN_OK) {
		sif::error << "Service 3: Could not deSerialize "
				 "into HK definiton object" << std::endl;
	}
	return result;
}


/* Generation functions */

ReturnValue_t Service3HousekeepingPSB::generateHkPacket(sid32_t sid,
		HkPacketDefinition * hkDefinition, uint8_t subservice) {
	// A PoolRawAccessHelper class is used to serialize
	// the HK packet with the pool ID list
	uint32_t * poolIdBuffer = hkDefinition->getParameters();
	uint8_t * sendBuffer = this->sendBuffer;
	uint8_t numberOfParameters = hkDefinition->getNumberOfParameters();
	size_t hkDataSize = 0;

	PoolRawAccessHelper poolRawAccess(poolIdBuffer, numberOfParameters);
	ReturnValue_t result = poolRawAccess.serializeWithValidityMask(
			&sendBuffer,&hkDataSize, sid::MAX_LARGE_HK_BUFFER_SIZE,
			SerializeIF::Endianness::BIG);
	if(result != RETURN_OK) {
		sif::debug << "Service 3: Pool Raw Access Serialization Issue" << std::endl;
		return result;
	}
	sendHkPacket(sid,hkDataSize, isDiagnostics, subservice);
	return RETURN_OK;
}

void Service3HousekeepingPSB::sendHkPacket(sid32_t sid, uint32_t hkDataSize,
		bool isDiagnostics, uint8_t subservice) {
	// Initiate the dummy HK packet and correct endianness of SID
	// before sending packet.
	ReturnValue_t result;
	HkParameterReport hkPacket(sid,sendBuffer);
	sid = EndianConverter::convertBigEndian(sid);
	TmPacketStored tmPacket(apid::SOURCE_OBSW, pus::PUS_SERVICE_3,
			subservice, packetSubCounter++, hkPacket.hkBuffer, hkDataSize,
			reinterpret_cast<const uint8_t *>(&sid),sizeof(sid));
	result = tmPacket.sendPacket(requestQueue->getDefaultDestination(),
			requestQueue->getId());
	if(result != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "Service 3: Sending HK packet failed !" << std::endl;
		updateErrorMap(sid,result);
	}
}


/* Update functions */

ReturnValue_t Service3HousekeepingPSB::updateHkPoolEntry(store_address_t storeId,
		HkPacketDefinition *hkDefinition) {
	size_t size = 0;
	uint8_t * modifyBuffer = hkBuffer;
	ReturnValue_t result = RETURN_FAILED;
	result = hkPool.modifyData(storeId, &modifyBuffer,&size);
	if(result != RETURN_OK) {
		sif::error << "Service 3: Error modifying HK pool file" << std::endl;
		return result;
	}
	size_t serializedSize = 0;
	result = hkDefinition->serialize(&modifyBuffer, &serializedSize,
	        sizeof(hkBuffer), SerializeIF::Endianness::MACHINE);
	if(result != RETURN_OK) {
		sif::debug << "Service 3: Could not serialize modified "
				 "HK definition object" << std::endl;
		return result;
	}
	if(size != serializedSize) {
		sif::error << "Service 3: Size missmatch"
				 " when updating HK definition" << std::endl;
		result = SIZE_MISSMATCH;
	}

	return result;
}


/* Delete functions */

ReturnValue_t Service3HousekeepingPSB::deleteReportStructure(uint32_t sid,
		SidToStoreIdMap * sidMap) {
	SidToStoreIdIterator sidIterator = sidMap->find(sid);
	if(sidIterator != sidMap->end()) {
		ReturnValue_t result = hkPool.deleteData(sidIterator.second());
		if (result != RETURN_OK) {
			return result;
		}
		result = sidMap->erase(&sidIterator);
		return result;
	}
	else {
		sif::error << "Service 3: SID not found" << std::endl;
		triggerEvent(SID_NOT_FOUND,sid,currentPacket.getSubService());
		return RETURN_FAILED;
	}
}

//void Service3HousekeepingPSB::poolIdSwapper(uint32_t * poolIdBuffer,
//		uint32_t poolIdBufferLength) {
//	uint32_t tempPoolIdBuffer[poolIdBufferLength];
//	memcpy(tempPoolIdBuffer,poolIdBuffer,poolIdBufferLength * 4);
//	EndianConverter::convertBigEndian(poolIdBuffer,
//	        tempPoolIdBuffer,poolIdBufferLength);
//}
