#include <mission/assembly/GPSAssembly.h>
#include <mission/devices/GPSHandler.h>


GPSAssembly::GPSAssembly(object_id_t objectId, object_id_t objectIdGps0,
        object_id_t objectIdGps1, object_id_t parentId):
		AssemblyBase(objectId, parentId), gps0Id(objectIdGps0),
		gps1Id(objectIdGps1) {
	ModeListEntry entry;
	entry.setObject(objectIdGps0);
	entry.setMode(MODE_OFF);
	entry.setSubmode(SUBMODE_NONE);
	entry.setInheritSubmode(false);

	modeTable.insert(entry);

	entry.setObject(objectIdGps1);
	entry.setMode(MODE_OFF);
	entry.setSubmode(SUBMODE_NONE);
	entry.setInheritSubmode(false);

	modeTable.insert(entry);

}

GPSAssembly::~GPSAssembly() {
}

ReturnValue_t GPSAssembly::commandChildren(Mode_t mode, Submode_t submode) {
	uint8_t d1 = static_cast<uint8_t>(Device::ONE);
	uint8_t d2 = static_cast<uint8_t>(Device::TWO);

	if(mode == MODE_ON) {
		if(submode == DUAL) {
			modeTable[d1].setMode(mode);
			// GPS Handler does not have submode (at the moment)
			// TODO: Add submodes in handler?
			modeTable[d1].setSubmode(submode);
			modeTable[d2].setMode(mode);
			modeTable[d2].setSubmode(submode);
		}
		if(submode == SINGLE) {
			// GPS Handler does not have submode (at the moment)
		    // TODO: Add submodes in handler?
			modeTable[d1].setMode(MODE_OFF);
			modeTable[d1].setSubmode(SUBMODE_NONE);
			modeTable[d2].setMode(MODE_OFF);
			modeTable[d2].setSubmode(SUBMODE_NONE);

			if(isUseable(gps0Id, mode)){
				modeTable[d1].setMode(mode);
				modeTable[d1].setSubmode(submode);
			}
			else {
				modeTable[d2].setMode(mode);
				modeTable[d2].setSubmode(submode);
			}
		}
	}
	else {
	    modeTable[d1].setMode(mode);
	    modeTable[d1].setSubmode(submode);
	    modeTable[d2].setMode(mode);
	    modeTable[d2].setSubmode(submode);
	}

	HybridIterator<ModeListEntry> tableIter(modeTable.begin(),modeTable.end());
	executeTable(tableIter);
	return RETURN_OK;
}

ReturnValue_t GPSAssembly::checkChildrenStateOn(Mode_t wantedMode,
		Submode_t wantedSubmode) {
	Mode_t gps0mode = childrenMap[gps0Id].mode;
	Mode_t gps1mode = childrenMap[gps1Id].mode;
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

ReturnValue_t GPSAssembly::isModeCombinationValid(Mode_t mode,
		Submode_t submode) {
	if(mode == MODE_ON and (submode != SINGLE or submode != DUAL)) {
		return INVALID_SUBMODE;
	}
	if(mode == DeviceHandlerIF::MODE_NORMAL) {
		return INVALID_MODE;
	}
	return RETURN_OK;
}


ReturnValue_t GPSAssembly::initialize() {
	ReturnValue_t result = AssemblyBase::initialize();
	if(result != RETURN_OK) {
		return result;
	}

	GPSHandler* gps1Handler = ObjectManager::instance()->get<GPSHandler>(gps0Id);
	GPSHandler* gps2Handler = ObjectManager::instance()->get<GPSHandler>(gps1Id);
	if((gps1Handler == nullptr) or (gps2Handler == nullptr)) {
	    return HasReturnvaluesIF::RETURN_FAILED;
	}

	result = registerChild(gps0Id);
	if (result != RETURN_OK) {
		return result;
	}
	result = registerChild(gps1Id);
	if (result != RETURN_OK) {
		return result;
	}
	return RETURN_OK;
}

bool GPSAssembly::isUseable(object_id_t object,Mode_t mode) {
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
