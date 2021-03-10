/*
 * EtlMultiMapWrapper.h
 *
 *  Created on: Jan 10, 2021
 *      Author: Jan Gerhards
 */
#include <mission/memory/tmstore/EtlMultiMapWrapperIF.h>

#ifndef MISSION_MEMORY_TMSTORE_ETLMULTIMAPWRAPPER_H_
#define MISSION_MEMORY_TMSTORE_ETLMULTIMAPWRAPPER_H_

template<typename TKey, typename TMapped, size_t SIZE>
class EtlMultiMapWrapper : public EtlMultiMapWrapperIF<TKey, TMapped> {
	etl::multimap<TKey, TMapped, SIZE> multimap;

public:
	EtlMultiMapWrapper();
	virtual ~EtlMultiMapWrapper();

	virtual ReturnValue_t insert(TKey key, TMapped value) override {
		if(multimap.full()) {
			return HasReturnvaluesIF::RETURN_FAILED;
		} else {
			multimap.insert(std::pair<TKey, TMapped>(key, value));  //todo: does this allocate storage dynamically? Check doc
			return HasReturnvaluesIF::RETURN_OK;
		}

		//test.insert(std::pair<int, int>(1, 2));
		//test[5] = 7;
		//test.erase(1);
	}

	virtual ReturnValue_t erase(TKey key, TMapped value) override {
		if(multimap.count(key) == 0) {  //etl multimap has no contains function, so this check has the same result as not contains
			return HasReturnvaluesIF::RETURN_FAILED;
		} else {
			std::pair<iterator, iterator> iterpair = multimap.equal_range(key);
			iterator it = iterpair.first;
			for(; it != iterpair.second; ++it) {
				if(it->second == value) {
					multimap.erase(it);
					break;
				}
			}
			return HasReturnvaluesIF::RETURN_OK;
		}
	}

	virtual MultiMapGetReturn<TKey, TMapped> get(TKey key) override {
		MultiMapGetReturn<TKey, TMapped> errorReturn = MultiMapGetReturn<TKey, TMapped>();
		errorReturn.returnValue = HasReturnvaluesIF::RETURN_FAILED;  //other fields do not have to be initialized, return_failed tells us all we need to know

		if(multimap.count(key) == 0) {  //etl multimap has no contains function, so this check has the same result as not contains
			return errorReturn;
		} else {
			pair<const_iterator,const_iterator> iterators = multimap.equal_range(key);
			MultiMapGetReturn<TKey, TMapped> successReturn = MultiMapGetReturn<TKey, TMapped>();
			successReturn.returnValue = HasReturnvaluesIF::RETURN_OK;
			successReturn.begin = iterators.first;
			successReturn.end = iterators.second;
			return successReturn;
		}

	}

	virtual void clear() override {
		multimap.clear();
	}
};

#endif /* MISSION_MEMORY_TMSTORE_ETLMULTIMAPWRAPPER_H_ */
