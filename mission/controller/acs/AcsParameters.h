/**
 * @author Mikael Senger
 *
 * About:
 */
#ifndef MISSION_CONTROLLER_ACS_ACSPARAMETERS_H_
#define MISSION_CONTROLLER_ACS_ACSPARAMETERS_H_

#include <fsfw/parameters/HasParametersIF.h>

class AcsParameters: public HasParametersIF {
public:

    AcsParameters(uint8_t parameterModuleId);

    virtual ~AcsParameters();

    virtual ReturnValue_t getParameter(uint8_t domainId, uint8_t uniqueIdentifier,
            ParameterWrapper *parameterWrapper, const ParameterWrapper *newValues,
            uint16_t startAtIndex);

    uint32_t testParam;

    struct SafeModeParameters {
        float testSafeParamOne;
        uint8_t testSafeParamTwo;
    } safeModeParameters;

    struct PointingModeParameters {
        float testPointParamOne[3];
        uint8_t testPointParamTwo;
    } pointingModeParameters;

    struct KalmanFilterParameters {
        float testKalmanParamOne[3][3];
        uint8_t testKalmanParamTwo;
    } kalmanFilterParameters;


private:
    const uint8_t parameterModuleId;
};

#endif /* MISSION_CONTROLLER_ACS_ACSPARAMETERS_H_ */
