#include "LedTask.h"

extern "C" {
#include <hal/Drivers/LED.h>
}

#include <cstdio>

LedTask::LedTask(object_id_t objectId, std::string name, LedModes ledMode):
        SystemObject(objectId), ledMode(ledMode) {
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
}

LedTask::~LedTask() {}


ReturnValue_t LedTask::performOperation(uint8_t operationCode) {
    LedTogglerFunc toggler;
    switch(ledMode) {
    case(LedModes::NONE): break;
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
}

bool LedTask::randomBool() {
  return std::rand() % 2 == 1;
}

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



