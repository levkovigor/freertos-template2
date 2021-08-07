#include <fsfw/ipc/QueueFactory.h>
#include "SatelliteSystem.h"

SatelliteSystem::SatelliteSystem(object_id_t setObjectId, object_id_t parent,
        uint32_t maxNumberOfSequences, uint32_t maxNumberOfTables,
        Mode_t noneMode, Mode_t bootMode, Mode_t safeMode, Mode_t idleMode):
        Subsystem(setObjectId, parent, maxNumberOfSequences, maxNumberOfTables),
        noneMode(noneMode), bootMode(bootMode), safeMode(safeMode),
        idleMode(idleMode) {
    eventQueue = QueueFactory::instance()->createMessageQueue(EVENT_MQ_DEPTH);
}

ReturnValue_t SatelliteSystem::initialize() {
    ReturnValue_t result = Subsystem::initialize();
    if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    EventManagerIF* eventManager = ObjectManager::instance()->get<EventManagerIF>(
            objects::EVENT_MANAGER);
    if (eventManager == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    // Subscribe for special events here which can change the state of
    // the whole satellite (e.g. temperature events)

    return result;
}

SatelliteSystem::~SatelliteSystem() {
    QueueFactory::instance()->deleteMessageQueue(eventQueue);
}

void SatelliteSystem::checkEventQueue() {
    EventMessage event;
    ReturnValue_t result = eventQueue->receiveMessage(&event);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        return;
    }

    // Add event handling for special events here.
    switch (event.getEvent()) {

    }
}

