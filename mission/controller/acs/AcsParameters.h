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

private:
    const uint8_t parameterModuleId;
};
#endif /* MISSION_CONTROLLER_ACS_ACSPARAMETERS_H_ */
