#ifndef MISSION_MEMORY_TMSTORE_ETLIMAPWRAPPER_H_
#define MISSION_MEMORY_TMSTORE_ETLIMAPWRAPPER_H_

#include <etl/map.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <iterator>

/**
 * @brief
 * @details
 * tested were: adding elements, adding elements when map is full, erasing values,
 * readding previously erased values, and clearing the map.
 * @author		Jan Gerhards
 * @tparam TKey
 * @tparam TMapped
 */
template<typename TKey, typename TMapped>
class EtlIMapWrapper {
    etl::imap<TKey, TMapped> *map;

public:
    using EtlReturnPair = std::pair<ReturnValue_t, TMapped*>;
    using EtlMap = etl::imap<TKey, TMapped>;
    using EtlMapIter = typename EtlMap::iterator;
    using EtlMapConstIter = typename EtlMap::const_iterator;

    EtlIMapWrapper(etl::imap<TKey, TMapped>* mapPtr) {
        map = mapPtr;
    }
    ~EtlIMapWrapper() = default;

    ReturnValue_t emplace(TKey key, TMapped value) {
        if(map->full()) {
            return HasReturnvaluesIF::RETURN_FAILED;
        } else {
            (*map)[key] = value;
            return HasReturnvaluesIF::RETURN_OK;
        }
    }

    ReturnValue_t erase(TKey key) {
        //etl map has no contains function, so this check has the same result as not contains
        if(map->count(key) == 0) {
            return HasReturnvaluesIF::RETURN_FAILED;
        } else {
            map->erase(key);
            return HasReturnvaluesIF::RETURN_OK;
        }
    }

    int eraseByValue(TMapped value) {
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

    EtlReturnPair get(TKey key) {
        auto errorPair = std::pair<ReturnValue_t, TMapped*>(
                HasReturnvaluesIF::RETURN_FAILED, NULL);

        if(map->count(key) == 0) {
            return errorPair;
        } else {
            TMapped* valuePtr = &(map->at(key));
            auto successPair = std::pair<ReturnValue_t, TMapped*>();
            successPair.second = valuePtr;
            successPair.first = HasReturnvaluesIF::RETURN_OK;
            return successPair;
        }

    }

    void clear() {
        map->clear();
    }

    etl::imap<TKey, TMapped>* getMapPointer() {
        return map;
    }
};

#endif /* MISSION_MEMORY_TMSTORE_ETLIMAPWRAPPER_H_ */
