#include "Factory.h"

#include <systemObjectList.h>
#include <blloader/core/SerialReceiverTask.h>
#include <fsfw/storagemanager/PoolManager.h>
#include <cstdint>

/**
 * Build tasks by using SystemObject Interface (Interface).
 * Header files of all tasks must be included
 * Please note that an object has to implement the system object interface
 * if the nterface validity is checked or retrieved later by using the
 * get<TargetInterface>(object_id) function from the ObjectManagerIF.
 *
 * Framework objects are created first.
 *
 * @ingroup init
 */
void Factory::produce(void) {
	setStaticFrameworkObjectIds();

	/* Pool manager handles storage und mutexes */
	/* Data Stores. Currently reserving 9600 bytes of memory */
	uint16_t numberOfElements[4] = {100, 30, 20, 10};
	uint16_t sizeofElements[4] = {32, 64, 128, 265};
	new PoolManager<4>(objects::TC_STORE,sizeofElements, numberOfElements);

	new SerialReceiverTask(objects::SERIAL_RECEIVER_TASK);
}

void Factory::setStaticFrameworkObjectIds() {
}


