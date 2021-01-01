/*
 * EtlMapWrapperBase.h
 *
 *  Created on: Dec 20, 2020
 *      Author: Jan Gerhards
 */
#include <etl/map.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>

#ifndef MISSION_MEMORY_TMSTORE_ETLMAPWRAPPERBASE_H_
#define MISSION_MEMORY_TMSTORE_ETLMAPWRAPPERBASE_H_

template<typename TKey, typename TMapped>
class EtlMapWrapperBase {
	etl::map<int, int, 3> test;

public:
	virtual ReturnValue_t insert(TKey key, TMapped value) {
		test.insert(std::pair<int, int>(1, 2));
		test[5] = 7;
		test.erase(1);
	}

	virtual ReturnValue_t erase(TMapped value) = 0;
	virtual std::pair<ReturnValue_t, TMapped&> get(TKey key) = 0;
	virtual void clear() = 0;
};

#endif /* MISSION_MEMORY_TMSTORE_ETLMAPWRAPPERBASE_H_ */
