#ifndef MISSION_MEMORY_TMSTORE_ETLIMULTIMAPWRAPPER_H_
#define MISSION_MEMORY_TMSTORE_ETLIMULTIMAPWRAPPER_H_

#include <etl/multimap.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>

template<typename TKey, typename TMapped>
struct MultiMapGetReturn {
    ReturnValue_t returnValue;
    typename etl::imultimap<TKey, TMapped>::const_iterator end;
    typename etl::imultimap<TKey, TMapped>::const_iterator begin;
};

/**
 * @brief
 * @details
 * tested were: adding elements, adding elements when map is full,
 * erasing values, readding previously erased values, clearing the
 * map, adding values with the same key, and retrieving multiple
 * values with the same key.
 * @author      Jan Gerhards
 * @tparam TKey
 * @tparam TMapped
 */
template<typename TKey, typename TMapped>
class EtlIMultiMapWrapper{
    etl::imultimap<TKey, TMapped> *multimap;

public:
    EtlIMultiMapWrapper(etl::imultimap<TKey, TMapped>* multiMapPtr) {
        multimap = multiMapPtr;
    }
    ~EtlIMultiMapWrapper() = default;

    ReturnValue_t emplace(TKey key, TMapped value) {
        if(multimap->full()) {
            return HasReturnvaluesIF::RETURN_FAILED;
        } else {
            multimap->insert(std::pair<TKey, TMapped>(key, value));
            return HasReturnvaluesIF::RETURN_OK;
        }
    }

    MultiMapGetReturn<TKey, TMapped> get(TKey key) {
        MultiMapGetReturn<TKey, TMapped> errorReturn = MultiMapGetReturn<TKey, TMapped>();
        //other fields do not have to be initialized, return_failed tells us all we need to know
        errorReturn.returnValue = HasReturnvaluesIF::RETURN_FAILED;

        //etl multimap has no contains function, so this check has the same result as not contains
        if(multimap->count(key) == 0) {
            return errorReturn;
        } else {
            std::pair<typename etl::imultimap<TKey, TMapped>::const_iterator,
            typename etl::imultimap<TKey, TMapped>::const_iterator> iterators =
                    multimap->equal_range(key);
            MultiMapGetReturn<TKey, TMapped> successReturn = MultiMapGetReturn<TKey, TMapped>();
            successReturn.returnValue = HasReturnvaluesIF::RETURN_OK;
            successReturn.begin = iterators.first;
            successReturn.end = iterators.second;
            return successReturn;
        }
    }

    ReturnValue_t erase(TKey key, TMapped value) {
        //etl multimap has no contains function, so this check has the same result as not contains
        if(multimap->count(key) == 0) {
            return HasReturnvaluesIF::RETURN_FAILED;
        } else {
            std::pair<typename etl::imultimap<TKey, TMapped>::const_iterator,
            typename etl::imultimap<TKey, TMapped>::const_iterator> iterpair =
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

    void clear() {
        multimap->clear();
    }

    etl::imultimap<TKey, TMapped>* getMultiMapPointer() {
        return multimap;
    }
};

#endif /* MISSION_MEMORY_TMSTORE_ETLIMULTIMAPWRAPPER_H_ */
