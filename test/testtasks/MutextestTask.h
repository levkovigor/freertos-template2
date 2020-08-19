/*
 * MutextestTask.h
 *
 *  Created on: 19.07.2018
 *      Author: mohr
 */

#ifndef MISSION_MUTEXTESTTASK_H_
#define MISSION_MUTEXTESTTASK_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/ipc/MutexFactory.h>

/**
 * Start two of them with a little time difference and different periods to see mutex in action
 */

class MutextestTask: public SystemObject,  public ExecutableObjectIF {
public:

	virtual ReturnValue_t performOperation(uint8_t operationCode = 0);

	MutextestTask(const char *name, object_id_t setObjectId);
	virtual ~MutextestTask();
private:
	static MutexIF *mutex;
	const char * name;
	bool locked;
};

#endif /* MISSION_MUTEXTESTTASK_H_ */
