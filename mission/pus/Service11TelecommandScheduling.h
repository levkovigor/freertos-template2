#ifndef MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_
#define MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_

#include <fsfw/tmtcservices/PusServiceBase.h>
#include <etl/multimap.h>
#include <OBSWConfig.h>



class Service11TelecommandScheduling final: public PusServiceBase {
public:

    enum Subservice {
                ENABLE_SCHEDULING = 1,
                DISABLE_SCHEDULING = 2,
                RESET_SCHEDULING = 3,
                INSERT_ACTIVITY = 4,    // basic
                DELETE_ACTIVITY = 5,    // basic
                FILTER_ACTIVITY = 6,
                TIMESHIFT_ACTIVITY = 7,
                DETAIL_REPORT = 9,
                TIMEBASE_SCHEDULE_DETAIL_REPORT = 10,
                TIMESHIFT_ALL_SCHEDULE_ACTIVITIES = 15
            };


    Service11TelecommandScheduling(object_id_t objectId, uint16_t apid, uint8_t serviceId);
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

        void debugPrint(void) const {
        	sif::info << "TelecommandStruct{ seconds: " << this->seconds
        			<< "  storeId (raw): " << this->storeId.raw << "}" << std::endl;
        }
    };

    StorageManagerIF* tcStore = nullptr;

    /**
     * The telecommand map uses the exectution time as a Unix time stamp as
     * the key. This is mapped to a generic telecommand struct.
     */
    using TelecommandMap = etl::multimap<uint32_t, TelecommandStruct,
            config::MAX_STORED_TELECOMMANDS>;

    TelecommandMap telecommandMap;


    ReturnValue_t handleRequest_InsertActivity();

};



#endif /* MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_ */
