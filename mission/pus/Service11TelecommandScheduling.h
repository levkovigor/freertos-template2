#ifndef MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_
#define MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_

#include <fsfw/tmtcservices/PusServiceBase.h>
#include <etl/multimap.h>
#include <OBSWConfig.h>

class Service11TelecommandScheduling: public PusServiceBase {
public:
    Service11TelecommandScheduling(object_id_t objectId, uint16_t apid,
            uint8_t serviceId);
    virtual ~Service11TelecommandScheduling();

    /** PusServiceBase overrides */

    //from last meeting: these do not have to be virtual. (=>change + make class final)
    // also: I can use C++17 features if I want to

    virtual ReturnValue_t handleRequest(uint8_t subservice) override;
    virtual ReturnValue_t performService() override;
private:
    struct TelecommandStruct {
        dur_millis_t milliseconds;
        store_address_t storeId;
    };

    /**
     * The telecommand map uses the exectution time as a Unix time stamp as
     * the key. This is mapped to a generic telecommand struct.
     */
    using TelecommandMap = etl::multimap<uint32_t, struct TelecommandStruct,
            config::MAX_STORED_TELECOMMANDS>;
    TelecommandMap telecommandMap;

};



#endif /* MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_ */
