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
        testParam(0), parameterModuleId(parameterModuleId) {

    safeModeParameters.testSafeParamOne = 1.5;
    safeModeParameters.testSafeParamTwo = 10;

    pointingModeParameters.testPointParamOne[0] = 0.1;
    pointingModeParameters.testPointParamOne[1] = 0.2;
    pointingModeParameters.testPointParamOne[2] = 0.3;
    pointingModeParameters.testPointParamTwo = 5;

    kalmanFilterParameters.testKalmanParamOne[0][0] = -1;
    kalmanFilterParameters.testKalmanParamOne[1][0] = 0;
    kalmanFilterParameters.testKalmanParamOne[2][0] = 0;
    kalmanFilterParameters.testKalmanParamOne[0][1] = 0;
    kalmanFilterParameters.testKalmanParamOne[1][1] = 0;
    kalmanFilterParameters.testKalmanParamOne[2][1] = 1;
    kalmanFilterParameters.testKalmanParamOne[0][2] = 0;
    kalmanFilterParameters.testKalmanParamOne[1][2] = 1;
    kalmanFilterParameters.testKalmanParamOne[2][2] = 0;
    kalmanFilterParameters.testKalmanParamTwo = 50;
}

AcsParameters::~AcsParameters() {
}

ReturnValue_t AcsParameters::getParameter(uint8_t domainId, uint8_t uniqueIdentifier,
        ParameterWrapper *parameterWrapper, const ParameterWrapper *newValues,
        uint16_t startAtIndex) {

    if (domainId == parameterModuleId) {
        switch (uniqueIdentifier) {
        case 0:
            switch (uniqueIdentifier & 0xFF) {
            case 0x0:
                parameterWrapper->set(testParam);
            break;
            default:
                return INVALID_IDENTIFIER_ID;
            }
        break;
        case 0x1:
            switch (uniqueIdentifier & 0xFF) {
            case 0x0:
                parameterWrapper->set(safeModeParameters.testSafeParamOne);
            break;
            case 0x1:
                parameterWrapper->set(safeModeParameters.testSafeParamTwo);
            break;
            default:
                return INVALID_IDENTIFIER_ID;
            }
        break;
        case 0x2:
            switch (uniqueIdentifier & 0xFF) {
            case 0x0:
                parameterWrapper->setVector(pointingModeParameters.testPointParamOne);
            break;
            case 0x1:
                parameterWrapper->set(pointingModeParameters.testPointParamTwo);
            break;
            default:
                return INVALID_IDENTIFIER_ID;
            }
        break;
        case 0x3:
            switch (uniqueIdentifier & 0xFF) {
            case 0x0:
                parameterWrapper->setMatrix(kalmanFilterParameters.testKalmanParamOne);
            break;
            case 0x1:
                parameterWrapper->set(kalmanFilterParameters.testKalmanParamTwo);
            break;
            default:
                return INVALID_IDENTIFIER_ID;
            }
        break;
        default:
            return INVALID_IDENTIFIER_ID;
        }
        return HasReturnvaluesIF::RETURN_OK;
    } else {
        return INVALID_DOMAIN_ID;
    }
}
