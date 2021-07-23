#ifndef MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_
#define MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_

#include <fsfw/tmtcservices/PusServiceBase.h>
#include <etl/multimap.h>
#include <OBSWConfig.h>

#include <fsfw/tmtcservices/TmTcMessage.h>


/**
 * @brief: PUS-Service 11 - Telecommand scheduling.
 * Full documentation: ECSS-E-ST-70-41C, p. 168:
 * ST[11] time-based scheduling
 *
 * This service provides the capability to command pre-loaded
 * application processes (telecommands) by releasing them at their
 * due-time.
 * References to telecommands are stored together with their due-timepoints
 * and are released at their corresponding due-time.
 *
 * Necessary subservice functionalities are implemented.
 * Those are:
 * TC[11,4] activity insertion
 * TC[11,5] activity deletion
 * TC[11,7] activity time-shift
 *
 * Groups are not supported.
 * This service remains always enabled. Sending a disable-request has no effect.
 */
class Service11TelecommandScheduling final: public PusServiceBase {
public:

    enum Subservice {
                ENABLE_SCHEDULING = 1,
                DISABLE_SCHEDULING = 2,
                RESET_SCHEDULING = 3,
                INSERT_ACTIVITY = 4,
                DELETE_ACTIVITY = 5,
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
        uint64_t uid;
        uint32_t seconds;
        store_address_t storeAddr;
    };

    const uint32_t TIME_MARGIN = 5u;

    StorageManagerIF* tcStore = nullptr;
    AcceptsTelecommandsIF* tcRecipient = nullptr;
    MessageQueueId_t recipientMsgQueueId = 0;

    static constexpr uint16_t MAX_STORED_TELECOMMANDS = 500;
    /**
     * The telecommand map uses the exectution time as a Unix time stamp as
     * the key. This is mapped to a generic telecommand struct.
     */
    using TelecommandMap = etl::multimap<uint32_t, TelecommandStruct, MAX_STORED_TELECOMMANDS>;

    TelecommandMap telecommandMap;

    ReturnValue_t handleRequest_InsertActivity();
    ReturnValue_t handleRequest_DeleteActivity();
    ReturnValue_t handleRequest_TimeshiftActivity();

    /**
     * @brief De-serializes currentPacket and retrieves its de-serialized timestamp
     * @param timestamp     (out) de-serialized timestamp
     */
    ReturnValue_t GetDeserializedTimestamp(uint32_t& timestamp);

    /**
     * @brief Generates a UID from the currentPacket
     * @param uid (out) Generated UID from packet
     */
    void GetRequestIdFromCurrentPacket(uint64_t& uid);

    /**
     * @brief Copys the currentPacket into the tcStore
     * @param [out] new store address inside tcStore
     * @return status
     */
    ReturnValue_t ReStorePacket(store_address_t* addrNew);

};



#endif /* MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_ */
