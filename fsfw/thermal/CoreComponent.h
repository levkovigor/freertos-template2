#ifndef MISSION_CONTROLLERS_TCS_CORECOMPONENT_H_
#define MISSION_CONTROLLERS_TCS_CORECOMPONENT_H_

#include "../datapoolglob/GlobalDataSet.h"
#include "../datapoolglob/GlobalPoolVariable.h"
#include "../thermal/ThermalComponentIF.h"
#include "../thermal/AbstractTemperatureSensor.h"
#include "../thermal/ThermalModule.h"
#include "../thermal/ThermalMonitor.h"

// TODO: Documentaiton, how to use this? only use Thermal Component, which inherits core component?
class CoreComponent: public ThermalComponentIF {
public:
	struct Parameters {
		float lowerOpLimit;
		float upperOpLimit;
		float heaterOn;
		float hysteresis;
		float heaterSwitchoff;
	};

	static const uint16_t COMPONENT_TEMP_CONFIRMATION = 5;

	CoreComponent(object_id_t reportingObjectId, uint8_t domainId, uint32_t temperaturePoolId,
			uint32_t targetStatePoolId, uint32_t currentStatePoolId,
			uint32_t requestPoolId, GlobDataSet *dataSet,
			AbstractTemperatureSensor *sensor,
			AbstractTemperatureSensor *firstRedundantSensor,
			AbstractTemperatureSensor *secondRedundantSensor,
			ThermalModuleIF *thermalModule, Parameters parameters,
			Priority priority, StateRequest initialTargetState =
					ThermalComponentIF::STATE_REQUEST_OPERATIONAL);

	virtual ~CoreComponent();

	virtual HeaterRequest performOperation(uint8_t opCode);

	void markStateIgnored();

	object_id_t getObjectId();

	uint8_t getDomainId() const;

	virtual float getLowerOpLimit();

	ReturnValue_t setTargetState(int8_t newState);

	virtual void setOutputInvalid();

	virtual ReturnValue_t getParameter(uint8_t domainId, uint16_t parameterId,
			ParameterWrapper *parameterWrapper,
			const ParameterWrapper *newValues, uint16_t startAtIndex);

protected:

	AbstractTemperatureSensor *sensor;
	AbstractTemperatureSensor *firstRedundantSensor;
	AbstractTemperatureSensor *secondRedundantSensor;
	ThermalModuleIF *thermalModule;

	gp_float_t temperature;
	gp_int8_t targetState;
	gp_int8_t currentState;
	gp_uint8_t heaterRequest;

	bool isHeating;

	bool isSafeComponent;

	float minTemp;

	float maxTemp;

	Parameters parameters;

	ThermalMonitor temperatureMonitor;

	const uint8_t domainId;

	virtual float getTemperature();
	virtual State getState(float temperature, Parameters parameters,
			int8_t targetState);

	virtual void checkLimits(State state);

	virtual HeaterRequest getHeaterRequest(int8_t targetState,
			float temperature, Parameters parameters);

	virtual State getIgnoredState(int8_t state);

	void updateMinMaxTemp();

	virtual Parameters getParameters();
};

#endif /* MISSION_CONTROLLERS_TCS_CORECOMPONENT_H_ */
