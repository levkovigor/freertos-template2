#ifndef MISSION_DEVICES_MGMLIS3MDLHANDLER_H_
#define MISSION_DEVICES_MGMLIS3MDLHANDLER_H_

#include "devicedefinitions/MGMHandlerLIS3Definitions.h"

#include <fsfw/devicehandlers/DeviceHandlerBase.h>
#include <fsfw/globalfunctions/PeriodicOperationDivider.h>

#include <fsfwconfig/events/subsystemIdRanges.h>

/**
 * @brief   Device handler object for the LIS3MDL 3-axis magnetometer
 *          by STMicroeletronics
 * @details
 * Datasheet can be found online by googling LIS3MDL.
 * @author  L. Loidold, R. Mueller
 */
class MGMHandlerLIS3MDL: public DeviceHandlerBase {
public:

    enum class CommunicationStep {
        DATA,
        TEMPERATURE
    };

	static const uint8_t INTERFACE_ID = CLASS_ID::MGM_LIS3MDL;
	static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::MGM_LIS3MDL;
	//Notifies a command to change the setup parameters
	static const Event CHANGE_OF_SETUP_PARAMETER = MAKE_EVENT(0, SEVERITY::LOW);

	MGMHandlerLIS3MDL(uint32_t objectId, object_id_t deviceCommunication,
	        CookieIF* comCookie);
	virtual ~MGMHandlerLIS3MDL();

protected:

	/** DeviceHandlerBase overrides */
	virtual void doShutDown() override;
	virtual void doStartUp() override;
	virtual void doTransition(Mode_t modeFrom, Submode_t subModeFrom) override;
	virtual uint32_t getTransitionDelayMs(Mode_t from, Mode_t to) override;
	virtual ReturnValue_t buildCommandFromCommand(
			DeviceCommandId_t deviceCommand, const uint8_t *commandData,
			size_t commandDataLen) override;
	virtual ReturnValue_t buildTransitionDeviceCommand(
	        DeviceCommandId_t *id) override;
	virtual ReturnValue_t buildNormalDeviceCommand(
	        DeviceCommandId_t *id) override;
	virtual ReturnValue_t scanForReply(const uint8_t *start, size_t len,
	        DeviceCommandId_t *foundId, size_t *foundLen) override;
	virtual ReturnValue_t interpretDeviceReply(DeviceCommandId_t id,
			const uint8_t *packet) override;
	virtual void fillCommandAndReplyMap() override;
	virtual void modeChanged(void) override;
	void setNormalDatapoolEntriesInvalid() override;

private:

	/*------------------------------------------------------------------------*/
	/* Device specific commands and variables */
	/*------------------------------------------------------------------------*/
	/**
	 * Sets the read bit for the command
	 * @param single command to set the read-bit at
	 * @param boolean to select a continuous read bit, default = false
	 */
	uint8_t readCommand(uint8_t command, bool continuousCom = false);

	/**
	 * Sets the write bit for the command
	 * @param single command to set the write-bit at
	 * @param boolean to select a continuous write bit, default = false
	 */
	uint8_t writeCommand(uint8_t command, bool continuousCom = false);

	/**
	 * This Method gets the full scale for the measurement range
	 * e.g.: +- 4 gauss. See p.25 datasheet.
	 *  @return The ReturnValue does not contain the sign of the value
	 */
	uint8_t getFullScale(uint8_t ctrlReg2);

	/**
	 * The 16 bit value needs to be divided by the full range of a 16bit value
	 * and then multiplied with the current scale of the MGM.
	 * This factor returns the factor required to achieve this with
	 * one multiplication.
	 *
	 * @param scale is the return value of the getFulscale Method
	 * @return Multiplication factor to get the sensor value from raw data.
	 */
	float getSensitivityFactor(uint8_t scale);

	/**
	 * This Command detects the device ID
	 */
	ReturnValue_t identifyDevice();

	virtual void setupMgm();

	/*------------------------------------------------------------------------*/
	/* Non normal commands */
	/*------------------------------------------------------------------------*/
	/**
	 * Enables/Disables the integrated Temperaturesensor
	 * @param commandData On or Off
	 * @param length of the commandData: has to be 1
	 */
	virtual ReturnValue_t enableTemperatureSensor(const uint8_t *commandData,
			size_t commandDataLen);

	/**
	 * Sets the accuracy of the measurement of the axis. The noise is changing.
	 * @param commandData LOW, MEDIUM, HIGH, ULTRA
	 * @param length of the command, has to be 1
	 */
	virtual ReturnValue_t setOperatingMode(const uint8_t *commandData,
			size_t commandDataLen);


	//Length a sindgle command SPI answer
	static const uint8_t SINGLE_COMMAND_ANSWER_LEN = 2;

	//Single SPIcommand has 2 bytes, first for adress, second for content
	size_t singleComandSize = 2;
	//has the size for all adresses of the lis3mdl + the continous write bit
	uint8_t commandBuffer[MGMLIS3MDL::NR_OF_DATA_AND_CFG_REGISTERS + 1];

	/**
	 * We want to save the registers we set, so we dont have to read the
	 * registers when we want to change something.
	 * --> everytime we change set a register we have to save it
	 */
	uint8_t registers[MGMLIS3MDL::NR_OF_CTRL_REGISTERS];

	uint8_t statusRegister = 0;


	/**
	 * As this is a SPI Device, we get the Answer of the last sent command in
	 * the next read cycle, so we could check the command for identification.
	 */
	DeviceCommandId_t lastSentCommand = DeviceHandlerIF::NO_COMMAND;

	/**
	 * We always update all registers together, so this method updates
	 * the rawpacket and rawpacketLen, so we just manipulate the local
	 * saved register
	 *
	 */
	ReturnValue_t prepareCtrlRegisterWrite();

	enum InternalState {
		STATE_NONE, STATE_FIRST_CONTACT, STATE_SETUP, STATE_CHECK_REGISTERS
	};

	InternalState internalState = STATE_NONE;
	CommunicationStep communicationStep = CommunicationStep::DATA;
	bool commandExecuted = false;

#if OBSW_REDUCED_PRINTOUT == 0
    PeriodicOperationDivider* debugDivider;
#endif

};

#endif /* MISSION_DEVICES_MGMLIS3MDLHANDLER_H_ */
