#ifndef TEST_LEDTASK_H_
#define TEST_LEDTASK_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <cstdint>
#include <map>

/**
 * @brief   LED task.
 * @details
 * Right now, only works on iOBC and only should be included in the iOBC task.
 */
class LedTask: public SystemObject, public ExecutableObjectIF  {
public:
	enum LedModes: uint8_t {
		NONE,
		BLINKY,
		WAVE_UP,
		WAVE_DOWN,
		DISCO,
		RANDOM
	};

	LedTask(object_id_t objectId, std::string name, LedModes ledMode);
	virtual ~LedTask();

    virtual ReturnValue_t performOperation(uint8_t operationCode = 0) override;

    static void toggleBuiltInLed1();
    static void toggleBuiltInLed2();
    static void toggleBuiltInLed3();
    static void toggleBuiltInLed4();

    static bool randomBool();
protected:
    uint8_t modeCounter = 6;

    enum BuiltInLedIds {
        LED_1,
        LED_2,
        LED_3,
		LED_4
    };

    LedModes ledMode = LedModes::WAVE_DOWN;

    using LedTogglerFunc = void (*) ();
    std::map<uint8_t, LedTogglerFunc> ledMap;
    std::map<uint8_t, LedTogglerFunc>::iterator ledMapIter;
    std::map<uint8_t, LedTogglerFunc>::reverse_iterator ledMapRevIter;


    bool testFlag = false;
    uint8_t counter { 0 };
    uint8_t counterTrigger { 20 };
private:

    bool randMode = false;
    void switchModeRandom();
};

#endif /* TEST_LEDTASK_H_ */
