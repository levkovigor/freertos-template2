/*
 * EtlIMapWrapper.h
 *
 *  Created on: Jan 1, 2021
 *      Author: Jan Gerhards
 */
#include <etl/map.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <iterator>

#ifndef MISSION_MEMORY_TMSTORE_ETLIMAPWRAPPER_H_
#define MISSION_MEMORY_TMSTORE_ETLIMAPWRAPPER_H_

template<typename TKey, typename TMapped>
class EtlIMapWrapper {
	etl::imap<TKey, TMapped> *map;

public:
    using EtlReturnPair = std::pair<ReturnValue_t, TMapped*>;
    using EtlMap = etl::imap<TKey, TMapped>;
    using EtlMapIter = typename EtlMap::iterator;
    using EtlMapConstIter = typename EtlMap::const_iterator;

	EtlIMapWrapper();
	~EtlIMapWrapper();

	ReturnValue_t emplace(TKey key, TMapped value) override {
		if(map->full()) {
			return HasReturnvaluesIF::RETURN_FAILED;
		} else {
			(*map)[key] = value;  //todo: does this allocate storage dynamically? Check doc
			return HasReturnvaluesIF::RETURN_OK;
		}

		//test.insert(std::pair<int, int>(1, 2));
		//test[5] = 7;
		//test.erase(1);
	}

	ReturnValue_t erase(TKey key) override {
		if(map->count(key) == 0) {  //etl map has no contains function, so this check has the same result as not contains
			return HasReturnvaluesIF::RETURN_FAILED;
		} else {
			map->erase(key);
			return HasReturnvaluesIF::RETURN_OK;
		}
	}

	int eraseByValue(TMapped value) override {
		int numDeletedElements = 0;
		for(EtlMapConstIter iter = map->begin(); iter != map->end();) {
			if((iter->second) == value) {
				iter = map->erase(iter);
				numDeletedElements++;
			} else {
				iter++;
			}
		}
		return numDeletedElements;
	}

	EtlReturnPair get(TKey key) override {
		auto errorPair = std::pair<ReturnValue_t, TMapped*>(
		        HasReturnvaluesIF::RETURN_FAILED, NULL); //todo: check not allocated on heap

		if(map->count(key) == 0) {
			return errorPair;
		} else {
			TMapped* valuePtr = &(map->at(key));
			auto successPair = std::pair<ReturnValue_t, TMapped*> (
			        HasReturnvaluesIF::RETURN_OK, valuePtr); //todo: check not allocated on heap
			return successPair;
		}

	}

	void clear() override {
		map->clear();
	}

	void insertMapPtr(etl::imap<TKey, TMapped>* ptr) {
		map = ptr;
	}

	etl::imap<TKey, TMapped>* getMapPointer() {
		return map;
	}
};

#endif /* MISSION_MEMORY_TMSTORE_ETLIMAPWRAPPER_H_ */
