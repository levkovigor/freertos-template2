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

/*
 * This class is the base class for a EtlMapWrapper. It enables using multiple instances of EtlMapWrapper
 * with varying sizes in a common collection. To achieve this, create the Wrapper objects with their
 * respective sizes. After that, create a pointer to them that points to an object of this base class.
 * This can then be used without every pointer pointing to a slightly different class based on the sizes
 * of the maps.
 */
template<typename TKey, typename TMapped>
class EtlMapWrapperIF {  //todo: Constructor and Destructor needed?
public:
    using EtlReturnPair = std::pair<ReturnValue_t, TMapped*>;
    virtual ~EtlMapWrapperIF() {};

	virtual ReturnValue_t insert(TKey key, TMapped value) = 0;
	virtual ReturnValue_t erase(TKey key) = 0;
	virtual int eraseByValue(TMapped value) = 0;

	virtual EtlReturnPair get(TKey key) = 0;
	virtual void clear() = 0;
};

#endif /* MISSION_MEMORY_TMSTORE_ETLMAPWRAPPERBASE_H_ */
