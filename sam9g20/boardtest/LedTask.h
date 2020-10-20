#ifndef TEST_LEDTASK_H_
#define TEST_LEDTASK_H_

#include <fsfw/action/HasActionsIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <cstdint>
#include <map>
#include <string>

/**
 * @brief   LED task.
 * @details
 */
class LedTask: public SystemObject, public ExecutableObjectIF,
        public HasActionsIF {
public:
	enum LedModes: uint8_t {
		OFF,
		BLINKY,
		WAVE_UP,
		WAVE_DOWN,
		DISCO,
		RANDOM
	};

	LedTask(object_id_t objectId, std::string name, LedModes ledMode);
	virtual ~LedTask();

    ReturnValue_t performOperation(uint8_t operationCode = 0) override;
    ReturnValue_t initialize() override;

#ifdef ISIS_OBC_G20
    static void toggleBuiltInLed1();
    static void toggleBuiltInLed2();
    static void toggleBuiltInLed3();
    static void toggleBuiltInLed4();
#endif

    static bool randomBool();
protected:
    static constexpr ActionId_t ENABLE_LEDS = 0x00;
    static constexpr ActionId_t DISABLE_LEDS = 0x01;

    uint8_t modeCounter = 6;
    LedModes ledMode = LedModes::WAVE_DOWN;

    bool testFlag = false;
    uint8_t counter { 0 };
    uint8_t counterTrigger { 20 };

#ifdef ISIS_OBC_G20
    enum BuiltInLedIds {
        LED_1,
        LED_2,
        LED_3,
		LED_4
    };

    using LedTogglerFunc = void (*) ();
    std::map<uint8_t, LedTogglerFunc> ledMap;
    std::map<uint8_t, LedTogglerFunc>::iterator ledMapIter;
    std::map<uint8_t, LedTogglerFunc>::reverse_iterator ledMapRevIter;
#endif

private:

    /** HasActionIF overrides */
    MessageQueueId_t getCommandQueue() const override;
    ReturnValue_t executeAction(ActionId_t actionId,
            MessageQueueId_t commandedBy, const uint8_t* data,
            size_t size) override;

    MessageQueueIF* commandQueue;
    ActionHelper actionHelper;
    bool randMode = false;
    void switchModeRandom();
};

#endif /* TEST_LEDTASK_H_ */
