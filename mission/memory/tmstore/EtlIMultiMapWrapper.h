/*
 * EtlIMultiMapWrapper.h
 *
 *      Author: Jan Gerhards
 */

#include <etl/multimap.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>

#ifndef MISSION_MEMORY_TMSTORE_ETLIMULTIMAPWRAPPER_H_
#define MISSION_MEMORY_TMSTORE_ETLIMULTIMAPWRAPPER_H_

template<typename TKey, typename TMapped>
struct MultiMapGetReturn {
	ReturnValue_t returnValue;
	typename etl::imultimap<TKey, TMapped>::const_iterator end;
	typename etl::imultimap<TKey, TMapped>::const_iterator begin;
};

template<typename TKey, typename TMapped>
class EtlIMultiMapWrapper{
	etl::imultimap<TKey, TMapped> *multimap;

public:
	EtlIMultiMapWrapper();
	virtual ~EtlIMultiMapWrapper();

	ReturnValue_t emplace(TKey key, TMapped value) override {
		if(multimap->full()) {
			return HasReturnvaluesIF::RETURN_FAILED;
		} else {
			multimap->insert(std::pair<TKey, TMapped>(key, value));  //todo: does this allocate storage dynamically? Check doc
			return HasReturnvaluesIF::RETURN_OK;
		}
	}

	MultiMapGetReturn<TKey, TMapped> get(TKey key) {
		MultiMapGetReturn<TKey, TMapped> errorReturn = MultiMapGetReturn<TKey, TMapped>();
		errorReturn.returnValue = HasReturnvaluesIF::RETURN_FAILED;  //other fields do not have to be initialized, return_failed tells us all we need to know

		if(multimap->count(key) == 0) {  //etl multimap has no contains function, so this check has the same result as not contains
			return errorReturn;
		} else {
			std::pair<typename etl::imultimap<TKey, TMapped>::const_iterator, typename etl::imultimap<TKey, TMapped>::const_iterator> iterators
			  = multimap->equal_range(key);
			MultiMapGetReturn<TKey, TMapped> successReturn = MultiMapGetReturn<TKey, TMapped>();
			successReturn.returnValue = HasReturnvaluesIF::RETURN_OK;
			successReturn.begin = iterators.first;
			successReturn.end = iterators.second;
			return successReturn;
		}
	}

	ReturnValue_t erase(TKey key, TMapped value) override {
		if(multimap->count(key) == 0) {  //etl multimap has no contains function, so this check has the same result as not contains
			return HasReturnvaluesIF::RETURN_FAILED;
		} else {
			std::pair<typename etl::imultimap<TKey, TMapped>::const_iterator, typename etl::imultimap<TKey, TMapped>::const_iterator> iterpair =
					multimap->equal_range(key);
			typename etl::imultimap<TKey, TMapped>::const_iterator it = iterpair.first;
			for(; it != iterpair.second; ++it) {
				if(it->second == value) {
					multimap->erase(it);
					break;
				}
			}
			return HasReturnvaluesIF::RETURN_OK;
		}
	}

	void clear() override {
		multimap->clear();
	}

	void insertMultiMapPtr(etl::imultimap<TKey, TMapped>* ptr) {
		multimap = ptr;
	}

	etl::imultimap<TKey, TMapped>* getMultiMapPointer() {
		return multimap;
	}
};

#endif /* MISSION_MEMORY_TMSTORE_ETLIMULTIMAPWRAPPER_H_ */
