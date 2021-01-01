/*
 * EtlMapWrapper.h
 *
 *  Created on: Jan 1, 2021
 *      Author: Jan Gerhards
 */
#include <sourceobsw/mission/memory/tmstore/EtlMapWrapperBase.h>

#ifndef MISSION_MEMORY_TMSTORE_ETLMAPWRAPPER_H_
#define MISSION_MEMORY_TMSTORE_ETLMAPWRAPPER_H_

template<typename TKey, typename TMapped, size_t SIZE>
class EtlMapWrapper : public EtlMapWrapperBase<TKey, TMapped> {
	etl::map<TKey, TMapped, SIZE> map;

public:
	EtlMapWrapper();
	virtual ~EtlMapWrapper();
};

#endif /* MISSION_MEMORY_TMSTORE_ETLMAPWRAPPER_H_ */
