
/**
 * \file PCDUFailureIsolation.cpp
 *
 * \date 22.10.2019
 */

#include <mission/fdir/PCDUFailureIsolation.h>

PCDUFailureIsolation::PCDUFailureIsolation(object_id_t owner_,object_id_t parent_):
	DeviceHandlerFailureIsolation(owner_,parent_) {
}

PCDUFailureIsolation::~PCDUFailureIsolation() {
}

void PCDUFailureIsolation::decrementFaultCounters() {
}

ReturnValue_t PCDUFailureIsolation::eventReceived(EventMessage *event) {
	DeviceHandlerFailureIsolation::eventReceived(event);
	// any custom implementation here
	return RETURN_OK;
}
