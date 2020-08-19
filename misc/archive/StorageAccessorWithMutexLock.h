/**
 * @brief 	Helper classes to facilitate safe access to storages which is also
 * 			conforming to RAII principles
 * @details	These helper can be used together with the
 * 			StorageManager classes to manage access to a storage.
 * 			It can take care of thread-safety while also providing
 * 			mechanisms to automatically clear storage data and unlocking the
 * 			pool.
 */
#ifndef TEST_PROTOTYPES_STORAGEACCESSOR_H_
#define TEST_PROTOTYPES_STORAGEACCESSOR_H_

#include <fsfw/ipc/MutexHelper.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <memory>


/**
 * @brief   Accessor class which can be returned by pool managers
 *          or passed and set by pool managers to have safe access to the pool
 *          resources.
 */
class ConstStorageAccessor {
	//! StorageManager classes have exclusive access to private variables.
	template<uint8_t NUMBER_OF_POOLS>
	friend class PoolManager;
	template<uint8_t NUMBER_OF_POOLS>
	friend class LocalPool;
public:
	/**
	 * @brief	Simple constructor which takes the store ID of the storage
	 * 			entry to access.
	 * @param storeId
	 */
	ConstStorageAccessor(store_address_t storeId);

	/**
	 * @brief	Move ctor and move assignment allow returning accessors as
	 * 			a returnvalue. They prevent resource being free prematurely.
	 * Refer to: https://github.com/MicrosoftDocs/cpp-docs/blob/master/docs/cpp/
	 * move-constructors-and-move-assignment-operators-cpp.md
	 * @param
	 * @return
	 */
	ConstStorageAccessor& operator= (ConstStorageAccessor&&);
	ConstStorageAccessor (ConstStorageAccessor&&);

	//! The copy ctor and copy assignemnt should be deleted implicitely
	//! according to https://foonathan.net/2019/02/special-member-functions/
	//! but I still deleted them to make it more explicit. (remember rule of 5).
	ConstStorageAccessor& operator= (ConstStorageAccessor&) = delete;
	ConstStorageAccessor (ConstStorageAccessor&) = delete;

	/**
	 * @brief 	The destructor in default configuration takes care of
	 * 			deleting the accessed pool entry and unlocking the mutex
	 */
	virtual ~ConstStorageAccessor();

	/**
	 * @brief	Returns a pointer to the read-only data
	 * @return
	 */
	const uint8_t* data() const;

	/**
	 * @brief	Copies the read-only data to the supplied pointer
	 * @param pointer
	 */
	void getDataCopy(uint8_t *pointer);

	/**
	 * @brief   Calling this will prevent the Accessor from deleting the data
	 *          when the destructor is called.
	 */
	void release();
	/**
	 * @brief 	Locks the supplied mutex.
	 * @details
	 * The mutex will be unlocked automatically
	 * when this class is destroyed (for example when exiting the scope).
	 * Only one mutex can be locked at a time!
	 * @param mutex
	 * @param mutexTimeout
	 * @return
	 */
	ReturnValue_t lock(MutexIF* mutex,
			uint32_t mutexTimeout = MutexIF::NO_TIMEOUT);
	/**
	 * @brief	Unlocks the mutex (if one has been locked previously).
	 *          Unless this function is called, the mutex is unlocked
	 *          when the class exits the scope.
	 */
	void unlock();


	/**
	 * Get the size of the data
	 * @return
	 */
	size_t size() const;

	/**
	 * Get the storage ID.
	 * @return
	 */
	store_address_t getId() const;

	void print() const;
protected:
	const uint8_t* constDataPointer = nullptr;
	store_address_t storeId;
	size_t size_ = 0;
	//! Managing pool, has to assign itself.
	StorageManagerIF* store = nullptr;
	//! Unique pointer to the mutex lock instance. Is initialized by
	//! the pool manager.
	std::unique_ptr<MutexHelper> mutexLock = nullptr;
	bool deleteData = true;

	enum class AccessState {
		UNINIT,
		READ
	};
	//! Internal state for safety reasons.
	AccessState internalState = AccessState::UNINIT;
	/**
	 * Used by the pool manager instances to assign themselves to the
	 * accessor. This is necessary to delete the data when the acessor
	 * exits the scope ! The internal state will be considered read
	 * when this function is called, so take care all data is set properly as
	 * well.
	 * @param
	 */
	void assignStore(StorageManagerIF*);

};


/**
 * @brief   Child class for modifyable data. Also has a normal pointer member.
 */
class StorageAccessor: public ConstStorageAccessor {
	//! StorageManager classes have exclusive access to private variables.
	template<uint8_t NUMBER_OF_POOLS>
	friend class PoolManager;
	template<uint8_t NUMBER_OF_POOLS>
	friend class LocalPool;
public:
	StorageAccessor(store_address_t storeId);
	/**
	 * @brief	Move ctor and move assignment allow returning accessors as
	 * 			a returnvalue. They prevent resource being free prematurely.
	 * Refer to: https://github.com/MicrosoftDocs/cpp-docs/blob/master/docs/cpp/
	 * 			 move-constructors-and-move-assignment-operators-cpp.md
	 * @param
	 * @return
	 */
	StorageAccessor& operator= (StorageAccessor&&);
	StorageAccessor (StorageAccessor&&);

	ReturnValue_t write(uint8_t *data, size_t size,
		uint16_t offset);
	uint8_t* data();

private:
	//! Non-const pointer for modifyable data.
	uint8_t* dataPointer = nullptr;
	//! For modifyable data, the const pointer is assigned to the normal
	//! pointer by the pool manager so both access functions can be used safely
	void assignConstPointer();
};

#endif /* TEST_PROTOTYPES_STORAGEACCESSOR_H_ */
