#include "Ads1278.h"

Ads1278::Ads1278(addresses::LogAddr drdyPin): drdyPin(drdyPin) {
}

ReturnValue_t Ads1278::setMode(Modes mode) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Ads1278::getData(int32_t &currentValue, int32_t &voltageValue) {
    return HasReturnvaluesIF::RETURN_OK;
}

Ads1278::Modes Ads1278::getMode() const {
    return mode;
}

bool Ads1278::isDrdy(bool level, uint16_t timeoutValue) {
    return true;
}
