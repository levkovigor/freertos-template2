#ifndef MISSION_DEVICES_COREHANDLER_H_
#define MISSION_DEVICES_COREHANDLER_H_
#include <fsfw/controller/ControllerBase.h>

class CoreHandler: public ControllerBase {
public:
	//CoreHandler(object_id_t objectId);
    // We should track the HK variables. If some are out of order, power cycle
    // core (or iOBC via Vorago)


};



#endif /* MISSION_DEVICES_COREHANDLER_H_ */
