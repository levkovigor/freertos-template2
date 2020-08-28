#ifndef MISSION_ASSEMBLY_GYROASSEMBLY_H_
#define MISSION_ASSEMBLY_GYROASSEMBLY_H_

#include <fsfw/devicehandlers/AssemblyBase.h>
#include <fsfw/container/FixedArrayList.h>

/**
 * @brief Assembly object for all Gyro Devices.
 * @ingroup subsystems
 */
class GyroAssembly: public AssemblyBase {
public:
	GyroAssembly(object_id_t objectId, object_id_t objectIdGps0,
	        object_id_t objectIdGps1, object_id_t parentId);
	virtual ~GyroAssembly();

	/**
	 * command children to reach mode,submode
	 *
	 * set #commandsOutstanding correctly, or use executeTable()
	 *
	 * @param mode
	 * @param submode
	 * @return
	 *    - @c RETURN_OK if ok
	 *    - @c NEED_SECOND_STEP if children need to be commanded again
	 */
	virtual ReturnValue_t commandChildren(Mode_t mode, Submode_t submode);

	virtual ReturnValue_t checkChildrenStateOn(Mode_t wantedMode,
				Submode_t wantedSubmode);

	/**
	 * List of valid mode and submode combination for Gyro devices is
	 * implemented here
	 * @param mode
	 * @param submode
	 * @return
	 */
	virtual ReturnValue_t isModeCombinationValid(Mode_t mode,
				Submode_t submode);

	virtual ReturnValue_t initialize();
private:
    object_id_t gyro0Id;
    object_id_t gyro1Id;

	enum class Device {
		ONE,
		TWO
	};

	static constexpr uint8_t NUMBER_OF_GYRO_DEVICES = 2;
	FixedArrayList<ModeListEntry, NUMBER_OF_GYRO_DEVICES> modeTable;

	static const Submode_t SINGLE = 0;
	static const Submode_t DUAL = 1;

	bool isUseable(object_id_t object, Mode_t mode);

	ReturnValue_t handleNormalModeCommand(Mode_t mode, Submode_t submode,
	        uint8_t d0, uint8_t d1);
	ReturnValue_t handleOnModeCommand(Mode_t mode, Submode_t submode,
	        uint8_t d0, uint8_t d1);
};



#endif /* MISSION_ASSEMBLY_GYROASSEMBLY_H_ */
