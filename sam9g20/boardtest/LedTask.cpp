#include "LedTask.h"
#include <fsfw/ipc/QueueFactory.h>

extern "C" {
#ifdef ISIS_OBC_G20
#include <hal/Drivers/LED.h>
#else
#include <led_ek.h>
#endif
}

#include <cstdio>
#include <cstdlib>

LedTask::LedTask(object_id_t objectId, std::string name, LedModes ledMode):
        SystemObject(objectId), ledMode(ledMode),
        commandQueue(QueueFactory::instance()->createMessageQueue()),
        actionHelper(this, commandQueue) {
#ifdef ISIS_OBC_G20
    LED_start();
    LED_wave(10);

    /* Add built-in LEDs to LED map.
    Additional LEDs (for example external ones) can be added to the map
    if needed. */
    ledMap.emplace(LED_1, toggleBuiltInLed1);
    ledMap.emplace(LED_2, toggleBuiltInLed2);
    ledMap.emplace(LED_3, toggleBuiltInLed3);
    ledMap.emplace(LED_4, toggleBuiltInLed4);

    ledMapIter = ledMap.begin();
    ledMapRevIter = ledMap.rbegin();
#else
    LED_Set(0);
    LED_Clear(1);
#endif
}

LedTask::~LedTask() {}


ReturnValue_t LedTask::performOperation(uint8_t operationCode) {
    CommandMessage message;
    ReturnValue_t result = commandQueue->receiveMessage(&message);
    if(result == HasReturnvaluesIF::RETURN_OK) {
        actionHelper.handleActionMessage(&message);
    }

    if(ledMode == LedModes::OFF) {
        return HasReturnvaluesIF::RETURN_OK;
    }

#ifdef ISIS_OBC_G20
    LedTogglerFunc toggler;
    switch(ledMode) {
    case(LedModes::OFF): break;
    case(LedModes::BLINKY): {
    	if(ledMapIter == ledMap.end()) {
    		ledMapIter = ledMap.begin();
    	}
    	toggler = ledMapIter->second;
    	if(toggler != nullptr) {
    		toggler();
    	}
    	break;
    }
    case(LedModes::WAVE_UP): {
    	if(ledMapIter == ledMap.end()) {
    		ledMapIter = ledMap.begin();
    	}
    	toggler = ledMapIter->second;
    	toggler();
    	ledMapIter ++;
    	break;
    }
    case(LedModes::WAVE_DOWN): {
    	if(ledMapRevIter == ledMap.rend()) {
    		ledMapRevIter = ledMap.rbegin();
    	}
    	toggler = ledMapRevIter->second;
    	toggler();
    	ledMapRevIter ++;
    	break;
    }
    case(LedModes::DISCO): {
    	for(const auto& led: ledMap) {
    		if(randomBool()) {
    			toggler = led.second;
    			toggler();
    		}
    	}
    	break;
    }
    case(LedModes::RANDOM): {
    	randMode = true;
    	switchModeRandom();
    	break;
    }
    default:
    	break;
    }

    counter ++;
    if(randMode and counter == counterTrigger)  {
    	ledMode = LedModes::RANDOM;
    	counter = 0;
    }
    return 0;
#else
    LED_Toggle(0);
    LED_Toggle(1);

    return HasReturnvaluesIF::RETURN_OK;
#endif
}

bool LedTask::randomBool() {
  return std::rand() % 2 == 1;
}

#ifdef ISIS_OBC_G20
void LedTask::toggleBuiltInLed1() {
	LED_toggle(led_1);
}

void LedTask::toggleBuiltInLed2() {
	LED_toggle(led_2);
}

void LedTask::toggleBuiltInLed3() {
	LED_toggle(led_3);
}

void LedTask::toggleBuiltInLed4() {
	LED_toggle(led_4);
}
#endif

MessageQueueId_t LedTask::getCommandQueue() const {
    return commandQueue->getId();
}

ReturnValue_t LedTask::executeAction(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t *data, size_t size) {
    switch(actionId) {
    case(ENABLE_LEDS): {
        // TODO: make it so led mode is sent as well.
        if(ledMode == LedModes::OFF) {
            ledMode = LedModes::WAVE_UP;
            resetLeds();
        }
        actionHelper.finish(commandedBy, actionId);
        break;
    }
    case(DISABLE_LEDS): {
        ledMode = LedModes::OFF;
        resetLeds();
        actionHelper.finish(commandedBy, actionId);
        break;
    }
    default: {
        return HasActionsIF::INVALID_ACTION_ID;
    }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t LedTask::initialize() {
    ReturnValue_t result = actionHelper.initialize(commandQueue);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    return SystemObject::initialize();
}

void LedTask::switchModeRandom() {
	// generates random number between 0 and modeCounter - 2
	uint8_t randNumber = std::rand() % (modeCounter - 2);
	if(randNumber == 0) {
		ledMode = LedModes::BLINKY;
	}
	else if(randNumber == 1) {
		ledMode = LedModes::WAVE_DOWN;
	}
	else if(randNumber == 2) {
		ledMode = LedModes::WAVE_UP;
	}
	else if(randNumber == 3) {
		ledMode = LedModes::DISCO;
	}
}

void LedTask::resetLeds() {
#ifdef ISIS_OBC_G20
    LED_dark(led_1);
    LED_dark(led_2);
    LED_dark(led_3);
    LED_dark(led_4);
#else
    // Set power LED.
    LED_Set(0);
    LED_Clear(1);
#endif
}
