/**
 * @author Mikael Senger
 *
 * About:
 */

#include <fsfw/globalfunctions/constants.h>
#include "AcsParameters.h"

#include <stddef.h>
#include <cmath>

AcsParameters::AcsParameters(uint8_t parameterModuleId) :
        parameterModuleId(parameterModuleId) {
}

AcsParameters::~AcsParameters() {
}

ReturnValue_t AcsParameters::getParameter(uint8_t domainId, uint8_t uniqueIdentifier,
        ParameterWrapper *parameterWrapper, const ParameterWrapper *newValues,
        uint16_t startAtIndex) {
    return HasReturnvaluesIF::RETURN_OK;
}
