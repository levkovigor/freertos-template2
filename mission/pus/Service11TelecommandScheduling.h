#ifndef MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_
#define MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_

#include <fsfw/tmtcservices/PusServiceBase.h>
#include <etl/multimap.h>
#include <OBSWConfig.h>

// from last meeting:
// I can use max. C++17


class Service11TelecommandScheduling final: public PusServiceBase {
public:
    Service11TelecommandScheduling(object_id_t objectId, uint16_t apid,
            uint8_t serviceId);
    ~Service11TelecommandScheduling();

    /** PusServiceBase overrides */ 
    ReturnValue_t handleRequest(uint8_t subservice) override;
    ReturnValue_t performService() override;

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
