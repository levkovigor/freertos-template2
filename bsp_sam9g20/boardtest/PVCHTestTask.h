#ifndef BSP_SAM9G20_BOARDTEST_PVCHTESTTASK_H_
#define BSP_SAM9G20_BOARDTEST_PVCHTESTTASK_H_

#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/tasks/ExecutableObjectIF.h"

class PVCHTestTask: public SystemObject,
        public ExecutableObjectIF {
public:
    PVCHTestTask(object_id_t objectId);

    ReturnValue_t initialize() override;
    ReturnValue_t performOperation(uint8_t opCode) override;
private:


};

#endif /* BSP_SAM9G20_BOARDTEST_PVCHTESTTASK_H_ */
