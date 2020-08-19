#ifndef MISSION_FDIR_PCDUFAILUREISOLATION_H_
#define MISSION_FDIR_PCDUFAILUREISOLATION_H_

#include <fsfw/devicehandlers/DeviceHandlerFailureIsolation.h>
#include <fsfw/objectmanager/SystemObject.h>

class PCDUFailureIsolation: public DeviceHandlerFailureIsolation {
public:
	PCDUFailureIsolation(object_id_t owner_,object_id_t parent_  = 0);
	virtual ~PCDUFailureIsolation();

private:
	virtual void decrementFaultCounters();
	virtual ReturnValue_t eventReceived(EventMessage* event);

};

#endif /* MISSION_FDIR_PCDUFAILUREISOLATION_H_ */
