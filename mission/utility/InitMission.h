#ifndef MISSION_UTILITY_INITMISSION_H_
#define MISSION_UTILITY_INITMISSION_H_

#include <fsfw/objectmanager/SystemObjectIF.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

namespace InitMission {

void printAddObjectError(const char* name, object_id_t objectId) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "InitMission::printAddError: Adding object " << name << " with object ID 0x"
            << std::hex << std::setfill('0') << std::setw(8) << objectId
            << " failed!" << std::dec << std::endl;
#else
    sif::printError("InitMission::printAddError: Adding object %s with object ID 0x%08x failed!\n" ,
            name, objectId);
#endif
}

}



#endif /* MISSION_UTILITY_INITMISSION_H_ */
