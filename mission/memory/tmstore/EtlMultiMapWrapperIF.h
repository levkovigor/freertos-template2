/*
 * EtlMultiMapWrapperBase.h
 *
 *  Created on: Jan 10, 2021
 *      Author: Jan Gerhards
 */
#include <etl/multimap.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>

#ifndef MISSION_MEMORY_TMSTORE_ETLMULTIMAPWRAPPERBASE_H_
#define MISSION_MEMORY_TMSTORE_ETLMULTIMAPWRAPPERBASE_H_

template<typename TKey, typename TMapped>
struct MultiMapGetReturn {
	ReturnValue_t returnValue;
	std::multimap::const_iterator begin;
	etl::multimap::const_iterator<TKey, TMapped> end;
};

template<typename TKey, typename TMapped>
class EtlMultiMapWrapperIF {

public:
	virtual ReturnValue_t insert(TKey key, TMapped value) = 0;
	virtual ReturnValue_t erase(TKey key, TMapped value) = 0;
	virtual MultiMapGetReturn<TKey, TMapped> get(TKey key) = 0;
	virtual void clear() = 0;
};


#endif /* MISSION_MEMORY_TMSTORE_ETLMULTIMAPWRAPPERBASE_H_ */
