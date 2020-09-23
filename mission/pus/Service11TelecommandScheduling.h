#ifndef MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_
#define MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_

#include <fsfw/tmtcservices/PusServiceBase.h>
#include <etl/multimap.h>
#include <config/tmtc/tmtcSize.h>

class Service11TelecommandScheduling: public PusServiceBase {
public:
    Service11TelecommandScheduling(object_id_t objectId, uint16_t apid,
            uint8_t serviceId);

    /** PusServiceBase overrides */
    virtual ReturnValue_t handleRequest(uint8_t subservice) override;
    virtual ReturnValue_t performService() override;
private:
    struct TelecommandStruct {
        store_address_t storeId;
    };

    using TelecommandMap = etl::multimap<uint32_t, struct TelecommandStruct,
            tmtcsize::MAX_STORED_TELECOMMANDS>;
    TelecommandMap telecommandMap;

};



#endif /* MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_ */
