#ifndef BSP_SAM9G20_DEVICES_PVCH_PVCHIF_H_
#define BSP_SAM9G20_DEVICES_PVCH_PVCHIF_H_

#include "returnvalues/classIds.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"

class PvchIF {
public:
    static constexpr ReturnValue_t ADC_DRDY_ERROR =
            HasReturnvaluesIF::makeReturnCode(CLASS_ID::PVCH, 0);

};

#endif /* BSP_SAM9G20_DEVICES_PVCH_PVCHIF_H_ */
