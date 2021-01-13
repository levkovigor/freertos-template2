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
    ReturnValue_t initialize() override;


private:
    struct TelecommandStruct {
        uint32_t seconds;
        store_address_t storeId;

        TelecommandStruct(uint32_t seconds, store_address_t storeId):
        seconds(seconds), storeId(storeId) { }
    };

    StorageManagerIF* tcStore = nullptr;

    /**
     * The telecommand map uses the exectution time as a Unix time stamp as
     * the key. This is mapped to a generic telecommand struct.
     */

    //NOTE: Is "UNIX timestamp = seconds since 1970, returned by std::time() for example?"

    using TelecommandMap = etl::multimap<uint32_t, TelecommandStruct,
            config::MAX_STORED_TELECOMMANDS>;

    TelecommandMap telecommandMap;

    // storage for the raw data to be received
    const uint8_t* pRawData = nullptr;
    size_t size = 0;

};



#endif /* MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_ */
