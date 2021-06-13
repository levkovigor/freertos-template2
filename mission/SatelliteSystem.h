#ifndef MISSION_SATELLITESYSTEM_H_
#define MISSION_SATELLITESYSTEM_H_

#include <fsfw/subsystem/Subsystem.h>

class SatelliteSystem: public Subsystem {
public:
    static constexpr uint8_t EVENT_MQ_DEPTH = 20;

    SatelliteSystem(object_id_t setObjectId, object_id_t parent,
            uint32_t maxNumberOfSequences, uint32_t maxNumberOfTables,
            Mode_t noneMode, Mode_t bootMode, Mode_t safeMode, Mode_t idleMode);
    virtual~ SatelliteSystem();

    ReturnValue_t initialize() override;
private:
    void checkEventQueue();

    MessageQueueIF* eventQueue;

    Mode_t noneMode = 0;
    Mode_t bootMode = 0;
    Mode_t safeMode = 0;
    Mode_t idleMode = 0;
};



#endif /* MISSION_SATELLITESYSTEM_H_ */
