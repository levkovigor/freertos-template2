#include <mission/assembly/GyroAssembly.h>
#include <mission/devices/GPSHandler.h>
#include <mission/devices/GyroHandler.h>


GyroAssembly::GyroAssembly(object_id_t objectId, object_id_t objectIdGyro0,
        object_id_t objectIdGyro1, object_id_t parentId):
		AssemblyBase(objectId, parentId), gyro0Id(objectIdGyro0),
		gyro1Id(objectIdGyro1) {
	ModeListEntry entry;
	entry.setObject(objectIdGyro0);
	entry.setMode(MODE_OFF);
	entry.setSubmode(SUBMODE_NONE);
	entry.setInheritSubmode(false);

	modeTable.insert(entry);

	entry.setObject(objectIdGyro1);
	entry.setMode(MODE_OFF);
	entry.setSubmode(SUBMODE_NONE);
	entry.setInheritSubmode(false);

	modeTable.insert(entry);

}

GyroAssembly::~GyroAssembly() {
}

ReturnValue_t GyroAssembly::commandChildren(Mode_t mode, Submode_t submode) {
    uint8_t d0 = static_cast<uint8_t>(Device::ONE);
    uint8_t d1 = static_cast<uint8_t>(Device::TWO);
    ReturnValue_t result = RETURN_OK;

    if(mode == DeviceHandlerIF::MODE_NORMAL) {
        result = handleNormalModeCommand(mode, submode, d0, d1);
    }

    else if(mode == MODE_ON) {
        result = handleOnModeCommand(mode, submode, d0, d1);
    }
    else {
        modeTable[d0].setMode(mode);
        modeTable[d0].setSubmode(submode);
        modeTable[d1].setMode(mode);
        modeTable[d1].setSubmode(submode);
    }
	HybridIterator<ModeListEntry> tableIter(modeTable.begin(),modeTable.end());
	executeTable(tableIter);
	return result;
}

ReturnValue_t GyroAssembly::handleNormalModeCommand(Mode_t mode,
        Submode_t submode, uint8_t d0, uint8_t d1) {
    ReturnValue_t result = RETURN_OK;
    Mode_t gyro0Mode = childrenMap[gyro0Id].mode;
    Mode_t gyro1Mode = childrenMap[gyro1Id].mode;

    modeTable[d0].setMode(MODE_OFF);
    modeTable[d0].setSubmode(SUBMODE_NONE);
    modeTable[d1].setMode(MODE_OFF);
    modeTable[d1].setSubmode(SUBMODE_NONE);
    if(submode == SINGLE) {
        if(isUseable(gyro0Id, mode)) {
            if(gyro0Mode != MODE_OFF) {
                modeTable[d0].setMode(mode);
                modeTable[d0].setSubmode(submode);
            }
            else {
                result = NEED_SECOND_STEP;
                modeTable[d1].setMode(MODE_ON);
                modeTable[d1].setSubmode(SUBMODE_NONE);
            }
        }
        else {
            if(gyro1Mode != MODE_OFF) {
                modeTable[d1].setMode(mode);
                modeTable[d1].setSubmode(submode);
            }
            else {
                result = NEED_SECOND_STEP;
                modeTable[d1].setMode(MODE_ON);
                modeTable[d1].setSubmode(SUBMODE_NONE);
            }
        }

    }
    if(submode == DUAL) {
        if(gyro0Mode != MODE_OFF) {
            modeTable[d0].setMode(mode);
            modeTable[d0].setSubmode(submode);
        }
        else {
            result = NEED_SECOND_STEP;
            modeTable[d0].setMode(MODE_ON);
            modeTable[d0].setSubmode(SUBMODE_NONE);
        }
        if(gyro1Mode != MODE_OFF) {
            modeTable[d1].setMode(mode);
            modeTable[d1].setSubmode(submode);
        }
        else {
            result = NEED_SECOND_STEP;
            modeTable[d1].setMode(MODE_ON);
            modeTable[d1].setSubmode(SUBMODE_NONE);
        }
    }
    return result;
}

ReturnValue_t GyroAssembly::handleOnModeCommand(Mode_t mode, Submode_t submode,
        uint8_t d0, uint8_t d1) {
    // Gyro will propably always be in single mode.
    if(submode == DUAL) {
        modeTable[d0].setMode(mode);
        modeTable[d0].setSubmode(submode);
        modeTable[d1].setMode(mode);
        modeTable[d1].setSubmode(submode);
    }
    if(submode == SINGLE) {
        modeTable[d0].setMode(MODE_OFF);
        modeTable[d0].setSubmode(SUBMODE_NONE);
        modeTable[d1].setMode(MODE_OFF);
        modeTable[d1].setSubmode(SUBMODE_NONE);

        if(isUseable(gyro0Id, mode)) {
            modeTable[d0].setMode(MODE_ON);
            modeTable[d0].setSubmode(SUBMODE_NONE);
        }
        else {
            modeTable[d1].setMode(MODE_ON);
            modeTable[d1].setSubmode(SUBMODE_NONE);
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t GyroAssembly::checkChildrenStateOn(Mode_t wantedMode,
		Submode_t wantedSubmode) {
	Mode_t gps0mode = childrenMap[gyro0Id].mode;
	Mode_t gps1mode = childrenMap[gyro1Id].mode;
	if (wantedMode == MODE_ON) {
		if (wantedSubmode == SINGLE) {
			if (gps0mode == MODE_OFF and gps1mode == MODE_OFF) {
				return NOT_ENOUGH_CHILDREN_IN_CORRECT_STATE;
			}
			else {
				return RETURN_OK;
			}
		}
		else if (wantedSubmode == DUAL) {
			if (gps0mode == MODE_ON and gps1mode == MODE_ON) {
				return RETURN_OK;
			}
			else {
				return NOT_ENOUGH_CHILDREN_IN_CORRECT_STATE;
			}
		}
		else {
			return INVALID_SUBMODE;
		}
	}
	return RETURN_OK;
}

ReturnValue_t GyroAssembly::isModeCombinationValid(Mode_t mode,
		Submode_t submode) {
	if(mode == MODE_ON and (submode != SINGLE or submode != DUAL)) {
		return INVALID_SUBMODE;
	}
	if(mode == DeviceHandlerIF::MODE_NORMAL) {
		return INVALID_MODE;
	}
	return RETURN_OK;
}


ReturnValue_t GyroAssembly::initialize() {
	ReturnValue_t result = AssemblyBase::initialize();
	if(result != RETURN_OK) {
		return result;
	}

	GyroHandler* gps1Handler = objectManager->get<GyroHandler>(gyro0Id);
	GyroHandler* gps2Handler = objectManager->get<GyroHandler>(gyro1Id);
	if((gps1Handler == nullptr) or (gps2Handler == nullptr)) {
	    return HasReturnvaluesIF::RETURN_FAILED;
	}

	result = registerChild(gyro0Id);
	if (result != RETURN_OK) {
		return result;
	}
	result = registerChild(gyro1Id);
	if (result != RETURN_OK) {
		return result;
	}
	return RETURN_OK;
}

bool GyroAssembly::isUseable(object_id_t object,Mode_t mode) {
	if(healthHelper.healthTable->isFaulty(object)){
		return false;
	}

	if(childrenMap[object].mode == mode){
		return true;
	}

	if(healthHelper.healthTable->isCommandable(object)){
		return true;
	}
	return false;
}
