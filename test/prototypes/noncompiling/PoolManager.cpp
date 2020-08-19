#include <fsfw/storagemanager/PoolManager.h>

PoolManager::PoolManager(object_id_t setObjectId,
		const LocalPoolConfig localPoolConfig):
		LocalPool(setObjectId, localPoolConfig, true) {
	mutex = MutexFactory::instance()->createMutex();
}


PoolManager::~PoolManager(void) {
	MutexFactory::instance()->deleteMutex(mutex);
}


ReturnValue_t PoolManager::reserveSpace(
		const size_t size, store_address_t* address, bool ignoreFault) {
	MutexHelper mutexHelper(mutex,MutexIF::NO_TIMEOUT);
	ReturnValue_t status = LocalPool::reserveSpace(size,
			address,ignoreFault);
	return status;
}


ReturnValue_t PoolManager::deleteData(
		store_address_t packet_id) {
	// debug << "PoolManager( " << translateObject(getObjectId()) <<
	//       " )::deleteData from store " << packet_id.pool_index <<
	//       ". id is "<< packet_id.packet_i#include <test/prototypes/StoreAccessWrapper.h>ndex << std::endl;
	MutexHelper mutexHelper(mutex,MutexIF::NO_TIMEOUT);
	ReturnValue_t status = LocalPool::deleteData(packet_id);
	return status;
}


ReturnValue_t PoolManager::deleteData(uint8_t* buffer,
		size_t size, store_address_t* storeId) {
	MutexHelper mutexHelper(mutex,MutexIF::NO_TIMEOUT);
	ReturnValue_t status = LocalPool::deleteData(buffer,
			size, storeId);
	return status;
}


ReturnValue_t PoolManager::modifyData(
		StorageAccessWrapper& storeAccessor) {
	uint8_t* pointer = storeAccessor.data();
	storeAccessor.lock(mutex);
	ReturnValue_t result = LocalPool::modifyData(
			storeAccessor.getId(), &pointer, storeAccessor.sizePtr());
	return result;
}



//
// PoolAccessHelper_t PoolManager::modifyDataWithLock(
//		store_address_t packet_id, uint8_t** packet_ptr, size_t* size) {
//	auto storageHelper = std::unique_ptr<StorageHelper>(
//			new StorageHelper(packet_id, this, mutex));
//	ReturnValue_t result = LocalPool::modifyData(packet_id,
//				packet_ptr, size);
//	return PoolAccessHelper_t(result, std::move(storageHelper));
//}

//
// ReturnValue_t PoolManager::modifyData(
//		store_address_t packet_id, uint8_t **packet_ptr, size_t *size) {
//	return HasReturnvaluesIF::RETURN_FAILED;
//}
