#ifndef UNITTEST_HOSTED_LOCALDATAPOOLTEST_H_
#define UNITTEST_HOSTED_LOCALDATAPOOLTEST_H_

#include <fsfw/datapoollocal/LocalDataPoolManager.h>
#include <fsfw/datapoollocal/LocalDataSet.h>
#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/objectmanager/SystemObjectIF.h>
#include <fsfw/datapoollocal/LocalPoolVariable.h>
#include <fsfw/datapoollocal/LocalPoolVector.h>
#include <cstdint>
#include <CatchDefinitions.h>


class LocalDataPoolDummy: public HasLocalDataPoolIF,
	public SystemObjectIF {
public:
	LocalDataPoolDummy(object_id_t objectId): dataPoolManager(this, nullptr),
			objectId(objectId)
	{
		// Initialize the HK pool entries.
		dataPoolManager.initializeHousekeepingPoolEntriesOnce();
	}

	virtual object_id_t getObjectId() const override  {
		return objectId;
	}

	virtual ReturnValue_t checkObjectConnections() override {
		return HasReturnvaluesIF::RETURN_OK;
	}

	virtual ReturnValue_t initialize() override {
		return HasReturnvaluesIF::RETURN_OK;
	}

	virtual void forwardEvent(Event event, uint32_t parameter1 = 0,
			uint32_t parameter2 = 0) const override {};

	virtual uint32_t getPeriodicOperationFrequency() const override {
		return 1;
	}

	virtual~ LocalDataPoolDummy() {};

	enum class PoolIds {
		POOL_VAR_1,
		POOL_VAR_2,
		POOL_VEC_1,
		POOL_VEC_2
	};

	virtual LocalPoolDataSetBase* getDataSetHandle(sid_t sid) override {
		if(sid.objectId == objectId and sid.ownerSetId == 0) {
			return nullptr;
		}
		else {
			sif::warning << "Invalid SID! Returning nullptr!" << std::endl;
			return nullptr;
		}
	}


protected:
	virtual MessageQueueId_t getCommandQueue() const override {
		return tconst::testQueueId;
	}
	virtual ReturnValue_t initializeLocalDataPool(
	        LocalDataPool& localDataPoolMap,
	        LocalDataPoolManager& poolManager) override {
		// This is usually done compile time. I won't check return values here
		// every time and will assume that the developer takes care of
		// specifying unique keys.
		localDataPoolMap.emplace(static_cast<int>(PoolIds::POOL_VAR_1),
				new PoolEntry<uint8_t>({false}));

		localDataPoolMap.emplace(static_cast<int>(PoolIds::POOL_VAR_2),
				new PoolEntry<float>({2.5243}));

		localDataPoolMap.emplace(static_cast<int>(PoolIds::POOL_VEC_1),
				new PoolEntry<uint16_t>({0,0,2}, 3));

		localDataPoolMap.emplace(static_cast<int>(PoolIds::POOL_VEC_2),
				new PoolEntry<double>({-4.294, 95252.23232}, 2, true));
		return HasReturnvaluesIF::RETURN_OK;
	}

	virtual LocalDataPoolManager* getHkManagerHandle() override {
		return &dataPoolManager;
	}



private:
	LocalDataPoolManager dataPoolManager;
	object_id_t objectId = 0;

	//! Can be used to access all pool variables at once.
	//StaticLocalDataSet<> completePoolSet;
	uint32_t completeSetId = 0;
public:
	//! These might be held by a child class. These have to be initialized
	//! after the dataset.
//	lp_uint8_t ownerBoolVar;
//	lp_float_t ownerFloatVar;
//	lp_vec_t<uint16_t, 3> ownerUint16Vec;
};

#endif /* UNITTEST_TESTFW_NEWTESTS_TESTTEMPLATE_H_ */
