#ifndef POOLMANAGER_H_
#define POOLMANAGER_H_

#include <fsfw/storagemanager/LocalPool.h>
#include <test/prototypes/StorageHelper.h>
#include <test/prototypes/StoreAccessWrapper.h>
#include <memory>

using PoolAccessHelper_t = std::pair<ReturnValue_t, std::unique_ptr<StorageHelper>>;

/**
 * @brief	The PoolManager class provides an intermediate data storage with
 * 			a fixed pool size policy for inter-process communication.
 * @details	Uses local pool calls but is thread safe by protecting the call
 * 			with a lock.
 */
class PoolManager : public LocalPool {
public:
	PoolManager(object_id_t setObjectId,
			const LocalPoolConfig localPoolConfig);

	//! @brief	In the PoolManager's destructor all allocated memory is freed.
	virtual ~PoolManager();

	//! @brief LocalPool overrides which also return a unique pointer
//	PoolAccessHelper_t modifyDataWithLock(store_address_t packet_id,
//			uint8_t** packet_ptr, size_t* size);
	ReturnValue_t modifyData(StorageAccessWrapper& storeAccessor);
//	ReturnValue_t getData(store_address_t packet_id,
//				StorageAccessWrapper& storeAccessor, size_t* size);
	//! @brief LocalPool overrides for thread-safety.
	ReturnValue_t deleteData(store_address_t) override;
	ReturnValue_t deleteData(uint8_t* buffer, size_t size,
			store_address_t* storeId = nullptr) override;
protected:
	ReturnValue_t reserveSpace(const size_t size, store_address_t* address,
			bool ignoreFault) override;

	/**
	 * @brief	The mutex is created in the constructor and makes
	 * 			access mutual exclusive.
	 * @details	Locking and unlocking is done during searching for free slots
	 * 			and deleting existing slots.
	 */
	MutexIF* mutex;
private:
	//ReturnValue_t modifyData(store_address_t packet_id, uint8_t** packet_ptr,
	//		size_t* size) override;
};

#endif /* POOLMANAGER_H_ */
