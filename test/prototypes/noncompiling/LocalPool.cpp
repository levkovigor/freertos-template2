#include <fsfw/storagemanager/LocalPool.h>
#include <cstring>

LocalPool::LocalPool(object_id_t setObjectId, LocalPoolConfig poolConfig,
		bool registered, bool spillsToHigherPools) :
		SystemObject(setObjectId, registered),  NUMBER_OF_POOLS(poolConfig.size()),
		internalErrorReporter(nullptr), spillsToHigherPools(spillsToHigherPools)
{
	uint16_t index = 0;
	for (auto poolCfg : poolConfig) {
		this->element_sizes[index] = poolCfg.first;
		this->n_elements[index] = poolCfg.second;
		store[index] = new uint8_t[n_elements[index] * element_sizes[index]];
		size_list[index] = new size_t[n_elements[index]];
		memset(store[index], 0x00, (n_elements[index] * element_sizes[index]));
		//TODO checkme
		memset(size_list[index], 0xFF,
				(n_elements[index] * sizeof(STORAGE_FREE)));
		index++;
	}
}

LocalPool::~LocalPool(void) {
	for (uint16_t n = 0; n < NUMBER_OF_POOLS; n++) {
		delete[] store[n];
		delete[] size_list[n];
	}
}


ReturnValue_t LocalPool::addData(store_address_t* storageId,
		const uint8_t* data, size_t size, bool ignoreFault) {
	ReturnValue_t status = reserveSpace(size, storageId, ignoreFault);
	if (status == RETURN_OK) {
		write(*storageId, data, size);
	}
	return status;
}

ReturnValue_t LocalPool::getData(
		store_address_t packet_id, const uint8_t** packet_ptr, size_t* size) {
	uint8_t* tempData = NULL;
	ReturnValue_t status = modifyData(packet_id, &tempData, size);
	*packet_ptr = tempData;
	return status;
}

ReturnValue_t LocalPool::modifyData(store_address_t packet_id,
		uint8_t** packet_ptr, size_t* size) {
	ReturnValue_t status = RETURN_FAILED;
	if (packet_id.pool_index >= NUMBER_OF_POOLS) {
		return ILLEGAL_STORAGE_ID;
	}
	if ((packet_id.packet_index >= n_elements[packet_id.pool_index])) {
		return ILLEGAL_STORAGE_ID;
	}
	if (size_list[packet_id.pool_index][packet_id.packet_index]
										!= STORAGE_FREE) {
		uint32_t packet_position = getRawPosition(packet_id);
		*packet_ptr = &store[packet_id.pool_index][packet_position];
		*size = size_list[packet_id.pool_index][packet_id.packet_index];
		status = RETURN_OK;
	} else {
		status = DATA_DOES_NOT_EXIST;
	}
	return status;
}

ReturnValue_t LocalPool::getFreeElement(store_address_t* storageId,
		const size_t size, uint8_t** p_data, bool ignoreFault) {
	ReturnValue_t status = reserveSpace(size, storageId, ignoreFault);
	if (status == RETURN_OK) {
		*p_data = &store[storageId->pool_index][getRawPosition(*storageId)];
	} else {
		*p_data = NULL;
	}
	return status;
}


void LocalPool::write(store_address_t packet_id, const uint8_t* data,
		size_t size) {
	uint8_t* ptr;
	uint32_t packet_position = getRawPosition(packet_id);

	//check size? -> Not necessary, because size is checked before calling this function.
	ptr = &store[packet_id.pool_index][packet_position];
	memcpy(ptr, data, size);
	size_list[packet_id.pool_index][packet_id.packet_index] = size;
}

uint32_t LocalPool::getRawPosition(store_address_t packet_id) {
	return packet_id.packet_index * element_sizes[packet_id.pool_index];
}

//Returns page size of 0 in case store_index is illegal
uint32_t LocalPool::getPageSize(uint16_t pool_index) {
	if (pool_index < NUMBER_OF_POOLS) {
		return element_sizes[pool_index];
	} else {
		return 0;
	}
}

ReturnValue_t LocalPool::reserveSpace(const size_t size,
		store_address_t* address, bool ignoreFault) {
	ReturnValue_t status = getPoolIndex(size, &address->pool_index);
	if (status != RETURN_OK) {
		sif::error << "LocalPool( " << std::hex << getObjectId() << std::dec
				<< " )::reserveSpace: Packet too large." << std::endl;
		return status;
	}
	status = findEmpty(address->pool_index, &address->packet_index);
	while (status != RETURN_OK && spillsToHigherPools) {
		status = getPoolIndex(size, &address->pool_index, address->pool_index + 1);
		if (status != RETURN_OK) {
			//We don't find any fitting pool anymore.
			break;
		}
		status = findEmpty(address->pool_index, &address->packet_index);
	}
	if (status == RETURN_OK) {
		// if (getObjectId() == objects::IPC_STORE && address->pool_index >= 3) {
		//	   debug << "Reserve: Pool: " << std::dec << address->pool_index <<
		//				" Index: " << address->packet_index << std::endl;
		// }

		size_list[address->pool_index][address->packet_index] = size;
	} else {
		if (!ignoreFault and internalErrorReporter != nullptr) {
			internalErrorReporter->storeFull();
		}
		// error << "LocalPool( " << std::hex << getObjectId() << std::dec
		// 			<< " )::reserveSpace: Packet store is full." << std::endl;
	}
	return status;
}

ReturnValue_t LocalPool::getPoolIndex(size_t packet_size, uint16_t* poolIndex,
		uint16_t startAtIndex) {
	for (uint16_t n = startAtIndex; n < NUMBER_OF_POOLS; n++) {
		//debug << "LocalPool " << getObjectId() << "::getPoolIndex: Pool: " <<
		//		n << ", Element Size: " << element_sizes[n] << std::endl;
		if (element_sizes[n] >= packet_size) {
			*poolIndex = n;
			return RETURN_OK;
		}
	}
	return DATA_TOO_LARGE;
}

ReturnValue_t LocalPool::findEmpty(uint16_t pool_index, uint16_t* element) {
	ReturnValue_t status = DATA_STORAGE_FULL;
	for (uint16_t foundElement = 0; foundElement < n_elements[pool_index];
			foundElement++) {
		if (size_list[pool_index][foundElement] == STORAGE_FREE) {
			*element = foundElement;
			status = RETURN_OK;
			break;
		}
	}
	return status;
}


ReturnValue_t LocalPool::deleteData(store_address_t packet_id) {
	//if (getObjectId() == objects::IPC_STORE && packet_id.pool_index >= 3) {
	//	debug << "Delete: Pool: " << std::dec << packet_id.pool_index << " Index: "
	//	         << packet_id.packet_index << std::endl;
	//}
	ReturnValue_t status = RETURN_OK;
	uint32_t page_size = getPageSize(packet_id.pool_index);
	if ((page_size != 0)
			&& (packet_id.packet_index < n_elements[packet_id.pool_index])) {
		uint16_t packet_position = getRawPosition(packet_id);
		uint8_t* ptr = &store[packet_id.pool_index][packet_position];
		memset(ptr, 0, page_size);
		//Set free list
		size_list[packet_id.pool_index][packet_id.packet_index] = STORAGE_FREE;
	} else {
		//pool_index or packet_index is too large
		sif::error << "LocalPool:deleteData failed." << std::endl;
		status = ILLEGAL_STORAGE_ID;
	}
	return status;
}


ReturnValue_t LocalPool::deleteData(uint8_t* ptr, size_t size,
		store_address_t* storeId) {
	store_address_t localId;
	ReturnValue_t result = ILLEGAL_ADDRESS;
	for (uint16_t n = 0; n < NUMBER_OF_POOLS; n++) {
		//Not sure if new allocates all stores in order. so better be careful.
		if ((store[n] <= ptr) && (&store[n][n_elements[n]*element_sizes[n]]) > ptr) {
			localId.pool_index = n;
			uint32_t deltaAddress = ptr - store[n];
			// Getting any data from the right "block" is ok.
			// This is necessary, as IF's sometimes don't point to the first
			// element of an object.
			localId.packet_index = deltaAddress / element_sizes[n];
			result = deleteData(localId);
			//if (deltaAddress % element_sizes[n] != 0) {
			//	error << "Pool::deleteData: address not aligned!" << std::endl;
			//}
			break;
		}
	}
	if (storeId != NULL) {
		*storeId = localId;
	}
	return result;
}


ReturnValue_t LocalPool::deleteDataNonLocking(store_address_t storeId) {
	return deleteData(storeId);
}

void LocalPool::clearStore() {
	for (uint16_t n = 0; n < NUMBER_OF_POOLS; n++) {
		//TODO checkme
		memset(sizeList[n], 0xff, (numberOfElements[n] * sizeof(STORAGE_FREE)));
	}
}

ReturnValue_t LocalPool::initialize() {
	ReturnValue_t result = SystemObject::initialize();
	if (result != RETURN_OK) {
		return result;
	}
	internalErrorReporter = objectManager->get<InternalErrorReporterIF>(
			objects::INTERNAL_ERROR_REPORTER);
	if (internalErrorReporter == NULL){
		return RETURN_FAILED;
	}

	//Check if any pool size is large than the maximum allowed.
	for (uint8_t count = 0; count < NUMBER_OF_POOLS; count++) {
		if (element_sizes[count] >= STORAGE_FREE) {
			sif::error << "LocalPool::initialize: Pool is too large! "
					"Max. allowed size is: " << (STORAGE_FREE - 1) << std::endl;
			return RETURN_FAILED;
		}
	}
	return RETURN_OK;
}

