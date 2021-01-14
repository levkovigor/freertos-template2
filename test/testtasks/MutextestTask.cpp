/*
 * MutextestTask.cpp
 *
 *  Created on: 19.07.2018
 *      Author: mohr
 */

#include <test/testtasks/MutextestTask.h>

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>

MutexIF * MutextestTask::mutex = nullptr;

MutextestTask::MutextestTask(const char *name, object_id_t setObjectId) :
		SystemObject(setObjectId), name(name), locked(false) {
	if (mutex == NULL) {
		mutex = MutexFactory::instance()->createMutex();
	}
}

ReturnValue_t MutextestTask::performOperation(uint8_t operationCode) {
	if (!locked){
		sif::info << name << ": locking..." << std::endl;
		ReturnValue_t result = mutex->lockMutex(MutexIF::TimeoutType::BLOCKING);
		sif::info << name << ": locked with " << (int) result << std::endl;
		if (result == HasReturnvaluesIF::RETURN_OK){
			locked = true;
		}
	} else {
		sif::info << name << ": releasing" << std::endl;
		mutex->unlockMutex();
		locked = false;
	}

	return HasReturnvaluesIF::RETURN_OK;
}

MutextestTask::~MutextestTask() {
// TODO Auto-generated destructor stub
}

