#include "../health/HealthTable.h"
#include "../serialize/SerializeAdapter.h"
#include "../ipc/MutexFactory.h"

HealthTable::HealthTable(object_id_t objectid) :
		SystemObject(objectid) {
	mutex = MutexFactory::instance()->createMutex();;

	mapIterator = healthMap.begin();
}

HealthTable::~HealthTable() {
	MutexFactory::instance()->deleteMutex(mutex);
}

ReturnValue_t HealthTable::registerObject(object_id_t object,
		HasHealthIF::HealthState initilialState) {
	if (healthMap.count(object) != 0) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	healthMap.insert(
			std::pair<object_id_t, HasHealthIF::HealthState>(object,
					initilialState));
	return HasReturnvaluesIF::RETURN_OK;
}

void HealthTable::setHealth(object_id_t object,
		HasHealthIF::HealthState newState) {
	mutex->lockMutex(MutexIF::BLOCKING);
	HealthMap::iterator iter = healthMap.find(object);
	if (iter != healthMap.end()) {
		iter->second = newState;
	}
	mutex->unlockMutex();
}

HasHealthIF::HealthState HealthTable::getHealth(object_id_t object) {
	HasHealthIF::HealthState state = HasHealthIF::HEALTHY;
	mutex->lockMutex(MutexIF::BLOCKING);
	HealthMap::iterator iter = healthMap.find(object);
	if (iter != healthMap.end()) {
		state = iter->second;
	}
	mutex->unlockMutex();
	return state;
}

uint32_t HealthTable::getPrintSize() {
	mutex->lockMutex(MutexIF::BLOCKING);
	uint32_t size = healthMap.size() * 5 + 2;
	mutex->unlockMutex();
	return size;
}

bool HealthTable::hasHealth(object_id_t object) {
	bool exits = false;
	mutex->lockMutex(MutexIF::BLOCKING);
	HealthMap::iterator iter = healthMap.find(object);
	if (iter != healthMap.end()) {
		exits = true;
	}
	mutex->unlockMutex();
	return exits;
}

void HealthTable::printAll(uint8_t* pointer, size_t maxSize) {
	mutex->lockMutex(MutexIF::BLOCKING);
	size_t size = 0;
	uint16_t count = healthMap.size();
	ReturnValue_t result = SerializeAdapter::serialize(&count,
			&pointer, &size, maxSize, SerializeIF::Endianness::BIG);
	HealthMap::iterator iter;
	for (iter = healthMap.begin();
			iter != healthMap.end() && result == HasReturnvaluesIF::RETURN_OK;
			++iter) {
		result = SerializeAdapter::serialize(&iter->first,
				&pointer, &size, maxSize, SerializeIF::Endianness::BIG);
		uint8_t health = iter->second;
		result = SerializeAdapter::serialize(&health, &pointer, &size,
				maxSize, SerializeIF::Endianness::BIG);
	}
	mutex->unlockMutex();
}

ReturnValue_t HealthTable::iterate(
		std::pair<object_id_t, HasHealthIF::HealthState> *value, bool reset) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	mutex->lockMutex(MutexIF::BLOCKING);
	if (reset) {
		mapIterator = healthMap.begin();
	}
	if (mapIterator == healthMap.end()) {
		result = HasReturnvaluesIF::RETURN_FAILED;
	}
	*value = *mapIterator;
	mapIterator++;
	mutex->unlockMutex();

	return result;
}
