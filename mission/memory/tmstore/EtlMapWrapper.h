/*
 * EtlMapWrapper.h
 *
 *  Created on: Jan 1, 2021
 *      Author: Jan Gerhards
 */
#include <mission/memory/tmstore/EtlMapWrapperIF.h>
#include <iterator>

#ifndef MISSION_MEMORY_TMSTORE_ETLMAPWRAPPER_H_
#define MISSION_MEMORY_TMSTORE_ETLMAPWRAPPER_H_

/*
 * This class is a wrapper for an etlMap of size SIZE. It offers some (but not all) methods of
 * the etlMap. For these methods, the wrapper contains error handling not available
 * in the map itself (for example due to exceptions being disabled). In order to use multiple
 * EtlMapWrapper objects in a collection, pointers to the base class EtlMapWrapperBase can be used.
 */
template<typename TKey, typename TMapped, size_t SIZE>
class EtlMapWrapper : public EtlMapWrapperIF<TKey, TMapped> {

public:
    using EtlReturnPair = typename EtlMapWrapperIF<TKey, TMapped>::EtlReturnPair;
    using EtlMap = etl::map<TKey, TMapped, SIZE>;
    using EtlMapIter = typename EtlMap::iterator;
    using EtlMapConstIter = typename EtlMap::const_iterator;

	EtlMapWrapper();
	virtual ~EtlMapWrapper();

	virtual ReturnValue_t insert(TKey key, TMapped value) override {
		if(map.full()) {
			return HasReturnvaluesIF::RETURN_FAILED;
		} else {
			map[key] = value;  //todo: does this allocate storage dynamically? Check doc
			return HasReturnvaluesIF::RETURN_OK;
		}

		//test.insert(std::pair<int, int>(1, 2));
		//test[5] = 7;
		//test.erase(1);
	}

	virtual ReturnValue_t erase(TKey key) override {
		if(map.count(key) == 0) {  //etl map has no contains function, so this check has the same result as not contains
			return HasReturnvaluesIF::RETURN_FAILED;
		} else {
			map.erase(key);
			return HasReturnvaluesIF::RETURN_OK;
		}
	}

	virtual int eraseByValue(TMapped value) override {
		int numDeletedElements = 0;
		for(EtlMapConstIter iter = map.begin(); iter != map.end();) {
			if((iter->second) == value) {
				iter = map.erase(iter);
				numDeletedElements++;
			} else {
				iter++;
			}
		}
		return numDeletedElements;
	}

	virtual EtlReturnPair get(TKey key) override {
		auto errorPair = std::pair<ReturnValue_t, TMapped*>(
		        HasReturnvaluesIF::RETURN_FAILED, NULL); //todo: check not allocated on heap

		if(map.count(key) == 0) {
			return errorPair;
		} else {
			TMapped* valuePtr = &(map.at(key));
			auto successPair = std::pair<ReturnValue_t, TMapped*> (
			        HasReturnvaluesIF::RETURN_OK, valuePtr); //todo: check not allocated on heap
			return successPair;
		}

	}

	virtual void clear() override {
		map.clear();
	}
private:
    EtlMap map;
};

#endif /* MISSION_MEMORY_TMSTORE_ETLMAPWRAPPER_H_ */
