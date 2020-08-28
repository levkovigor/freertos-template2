#ifndef MISSION_PUS_SERVICE3HOUSEKEEPING_H_
#define MISSION_PUS_SERVICE3HOUSEKEEPING_H_

#include <config/hk/sid.h>

#include <fsfw/storagemanager/LocalPool.h>
#include <fsfw/container/FixedMap.h>
#include <fsfw/tmtcservices/PusServiceBase.h>
#include <mission/pus/servicepackets/Service3PsbPackets.h>

/**
 * @brief   Manges spacecraft housekeeping reports and
 *          sends pool variables (temperature, GPS data ...) to ground.
 *
 * @details Full Documentation: ECSS-E70-41A or ECSS-E-ST-70-41C.
 * Implementation based on PUS-C
 *
 * The housekeeping service type provides means to control and adapt the
 * spacecraft reporting plan according to the mission phases.
 * The housekeeping service type provides the visibility of any
 * on-board parameters assembled in housekeeping parameter report structures
 * or diagnostic parameter report structures as required for the mission.
 * The parameter report structures used by the housekeeping service can
 * be predefined on-board or created when needed.
 *
 * This is the PusServiceBase implementation whic uses the global data pool.
 * Pre-defined HK packets are added in config/objects/factory.
 * @author 	R. Mueller
 * @ingroup pus_services
 */
class Service3HousekeepingPSB: public PusServiceBase {
public:

	Service3HousekeepingPSB(object_id_t object_id, uint16_t apid,
			uint8_t serviceId);
	virtual ~Service3HousekeepingPSB();

	/* PUS + FSFW */
	static constexpr uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::PUS_SERVICE_3;
	//! Problem when generating HK packets. Pool Entry was disabled.
	static constexpr Event POOL_ENTRY_DISABLED = MAKE_EVENT(0, SEVERITY::LOW); //!< P1: SID, P2: Last return code
	static constexpr Event SID_NOT_FOUND = MAKE_EVENT(1,SEVERITY::LOW); //!< SID not found. P1: SID, P2: Subservice

	static constexpr uint8_t INTERFACE_ID = CLASS_ID::PUS_SERVICE_3;

	static constexpr ReturnValue_t SID_ALREADY_EXISTS = MAKE_RETURN_CODE(0x01); //!< P1: SID
	static constexpr ReturnValue_t SID_STORE_MAP_FULL = MAKE_RETURN_CODE(0x02); //!< P1: SID, P2: Map Size
	static constexpr ReturnValue_t SIZE_MISSMATCH = MAKE_RETURN_CODE(0x02);

	/**
	 * Handles periodic, non-TC-triggered activities.
	 * For Service 3, this function generates the Housekeeping TM packets.
	 * @return	A success or failure code that does not trigger
	 * 			any kind of verification message.
	 */
	ReturnValue_t performService() override;

	/**
	 * Handles all TC-triggered activities
	 * which are specified by the ECSS PUS Standard.
	 * PUS Service 3 exposes a lot of subservices to modify the HK collection
	 * during run time.
	 * @return -@c RETURN_OK on success
	 *         - Error Code otherwise
	 */
	ReturnValue_t handleRequest(uint8_t subservice) override;

	/**
	 * @brief Add a HK packet definition to the HK definition pool.
	 * This interface function is public
	 * so it can be used to initialize pre-defined HK packets in the factory.cpp
	 * @param hkPacket HK packet initialized with the second constructor
	 * @param isDiagnostics Passed to determine target map for SID
	 * @return -@c RETURN_OK on success
	 */
	ReturnValue_t addPacketDefinitionToStore(HkPacketDefinition * hkPacket,
			bool isDiagnostics);

	enum Subservice {
		//!< [EXPORT] : [TC] Add a new structure to the housekeeping reports
		ADD_HK_REPORT_STRUCTURE = 1,
		//!< [EXPORT] : [TC] Add a new structure to the diagnostics reports
		ADD_DIAGNOSTICS_REPORT_STRUCTURE = 2,

		DELETE_HK_REPORT_STRUCTURE = 3, //!< [EXPORT] : [TC]
		DELETE_DIAGNOSTICS_REPORT_STRUCTURE = 4, //!< [EXPORT] : [TC]

		ENABLE_PERIODIC_HK_REPORT_GENERATION = 5, //!< [EXPORT] : [TC]
		DISABLE_PERIODIC_HK_REPORT_GENERATION = 6, //!< [EXPORT] : [TC]

		ENABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION = 7, //!< [EXPORT] : [TC]
		DISABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION = 8, //!< [EXPORT] : [TC]

		//! [EXPORT] : [TC] Report HK structure by supplying SID
		REPORT_HK_REPORT_STRUCTURES = 9,
		//! [EXPORT] : [TC] Report Diagnostics structure by supplying SID
		REPORT_DIAGNOSTICS_REPORT_STRUCTURES = 11,

		//! [EXPORT] : [TM] Report corresponding to Subservice 9 TC
		HK_DEFINITIONS_REPORT = 10,
		//! [EXPORT] : [TM] Report corresponding to Subservice 11 TC
		DIAGNOSTICS_DEFINITION_REPORT = 12,

		//! [EXPORT] : [TM] Core packet. Contains Housekeeping data
		HK_REPORT = 25,
		//! [EXPORT] : [TM] Core packet. Contains diagnostics data
		DIAGNOSTICS_REPORT = 26,

		/* PUS-C */
		GENERATE_ONE_PARAMETER_REPORT = 27, //!< [EXPORT] : [TC]
		GENERATE_ONE_DIAGNOSTICS_REPORT = 28, //!< [EXPORT] : [TC]

		APPEND_PARAMETERS_TO_PARAMETER_REPORT_STRUCTURE = 29, //!< [EXPORT] : [TC]
		APPEND_PARAMETERS_TO_DIAGNOSTICS_REPORT_STRUCTURE = 30, //!< [EXPORT] : [TC]

		MODIFY_PARAMETER_REPORT_COLLECTION_INTERVAL = 31, //!< [EXPORT] : [TC]
		MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL = 32, //!< [EXPORT] : [TC]

		/* not used for now */
		// APPLY_FUNCTIONAL_REPORTING_CONFIGURATION = 37
	};

private:
	using SidToStoreIdMap = FixedMap<sid32_t, store_address_t>;
	typedef std::pair<sid32_t,store_address_t> SidToStoreIdMapEntry;
	typedef SidToStoreIdMap::Iterator SidToStoreIdIterator;

	struct ErrorMapValue {
		uint8_t errorCounter;
		ReturnValue_t returnCode;
		bool isDiagnostics;

	};

	typedef FixedMap<sid32_t, ErrorMapValue> ErrorCounterMap;
	typedef std::pair<sid32_t,ErrorMapValue> ErrorCounterMapEntry;
	typedef ErrorCounterMap::Iterator ErrorCounterMapIterator;

	//! If there is an issue during periodic generation, trigger event every specified cycles
	static constexpr uint8_t CHECK_ERROR_MAP_TRIGGER = 25;
	static constexpr uint8_t MAX_ERROR_MAP_ENTRIES = 10;
	//! If error counter reaches that value, pool entry is disabled for periodic reporting
	static constexpr uint8_t ERROR_MAP_DISABLE_TRIGGER = 5;

	uint16_t packetSubCounter { 0 };
	uint16_t cycleCounter { 0 };

	/* State Variables */
	sid32_t currentSid { 0 };
	HkPacketDefinition currentHkPacket;
	//! Mark type of hk entry. Don't forget to set when handling requests !
	bool isDiagnostics = false;
	//! Error Map is only used for periodic activities. This bool is used to disable error handling functions
	bool commandedFromGround = false;

	/* This buffer stores the HK data before it is sent as a TM packet */
	static const uint16_t MAX_HK_DATA_SIZE = sid::MAX_LARGE_HK_BUFFER_SIZE;
	uint8_t sendBuffer[MAX_HK_DATA_SIZE];

	/* Local HK Pool to store HK data */
	static constexpr uint16_t NUMBER_OF_HK_ELEMENTS[2] = {
	        sid::MAX_NUMBER_OF_LARGE_HK_DEFINITIONS,
	        sid::MAX_NUMBER_OF_SMALL_HK_DEFINITIONS};
	static constexpr uint16_t SIZE_OF_HK_ELEMENTS[2] = {
	        sid::MAX_LARGE_HK_BUFFER_SIZE, sid::MAX_SMALL_HK_BUFFER_SIZE};
	LocalPool<2> hkPool;

	/* SID to Store ID maps */
	uint8_t intervalCounter;
	SidToStoreIdMap sidHkMap;
	SidToStoreIdMap sidDiagnosticsMap;

	/* This buffer is used to store the HK definitions buffers */
	uint8_t hkBuffer[sid::MAX_LARGE_HK_BUFFER_SIZE];

	/* Error counter map which is checker regualarly */
	ErrorCounterMap errCountMap;

	/* Periodic HK packet generation */
	void generateAllPackets();
	void generateHkPacketsFrom(SidToStoreIdMap * sidToPoolIdMap);
	void updateErrorMap(sid32_t currentSID, ReturnValue_t returnCode);
	void initializeErrorMapEntry(ErrorMapValue * currentError,
			ReturnValue_t returnCode);
	void checkErrorMap();
	void disableErrorMapEntry(ErrorCounterMapIterator * iter);
	ReturnValue_t disableHkReporting(uint32_t sid);
	ReturnValue_t disableDiagnosticsReporting(uint32_t sid);

	/**
	 *  This function and all subroutines perform the periodic generation and
	 *  sending of the actual housekeeping packet
	 * @param sid SID of the housekeeping packet
	 * @param storeId StoreID where the packet was stored to update cycle values
	 * @param hkDefinition Definition object of HK definition
	 * @param isDiagnostics Specifies which subservice will be used for packet generation
	 * @return
	 */
	void checkHkPacketGeneration(store_address_t storeId,
			HkPacketDefinition * hkDefinition);
	void handleHkPacketGeneration(uint32_t sid, HkPacketDefinition * hkDefinition);


	/* Functions to add HK packet definitions */
	/**
	 * Add deSerialized HK packet definition from ground to store
	 * @param hkPacketDefinition
	 * @param isDiagnosticsPacket
	 * @return
	 */
	ReturnValue_t addPacketDefinitionToStore(sid32_t sid, SidToStoreIdMap * sidMap,
				HkPacketDefinition * hkPacketDefinition);
	ReturnValue_t checkSidMap(uint32_t sid,SidToStoreIdMap * sidMap);
	ReturnValue_t handleSidMapInsertion(uint32_t sid,SidToStoreIdMap * sidMap,
			store_address_t storeId);


	/* Functions to retrieve HK packet definitions */
	/**
	 * Retrieve Hk packet definition by supplying a SID and the target map.
	 * #isDiagnostics needs to be set beforehand !
	 * @param sid
	 * @param storeId [out] optionally receive storeID
	 * @param hkPacket [out] HK packet definition
	 * @return - @c RETURN_OK on success
	 * 		   - @c ErrorCode otherwise
	 */
	ReturnValue_t retrieveHkPacketDefinitionFromSid(sid32_t sid,
			store_address_t * storeId, HkPacketDefinition * hkPacket);
	ReturnValue_t retrieveHkPacketDefinitionFromSid(sid32_t sid,
			HkPacketDefinition * hkPacket);
	/**
	 * Retrieve Hk packet definition by supplying a SID and the target map.
	 * @param hkPacket [out] HK packet definition
	 * @return - @c RETURN_OK on success
	 * 		   - @c ErrorCode otherwise
	 */
	ReturnValue_t retrieveHkPacketDefinition(store_address_t storeId,
	        HkPacketDefinition * hkPacket);

	/**
	 * Retrieve Store ID with provided SID from Map
	 * @param sid
	 * @param sidMap
	 * @param storeId [out] Found Store ID
	 * @return -@c RETURN_OK if store ID was found
	 *         -@c RETURN_FAILED otherwise, Event will be triggered too
	 */
	ReturnValue_t retrieveStoreID(uint32_t sid, SidToStoreIdMap * sidMap,
			store_address_t * storeId);

	/** Handles HK Definition retrieval from local pool */
	ReturnValue_t retrieveHkDefinitionBufferFromPool(const uint8_t ** recvBuffer,
			store_address_t storeId, size_t * size);
	/** Handles HK Definition buffer transformation into HK definition object */
	ReturnValue_t transformDefinitionBufferIntoObject(
			HkPacketDefinition * hkDefinition, const uint8_t ** recvBuffer,
			size_t * size);

	/**
	 * After the hk definition entries were updated (e.g. cycleCounter),
	 * the HK definition object is transformed back into a buffer and
	 * the respective HK pool entry is updated
	 * @param hkDefinition
	 * @param serializedSize
	 * @return
	 */
	ReturnValue_t updateHkPoolEntry(store_address_t storeId,
			HkPacketDefinition * hkDefinition);


	/* Subservice Handling */
	/**
	 * Check subservice and set #isDiagnostics flag
	 * @param subservice
	 * @return
	 */
	ReturnValue_t checkSubservice(uint8_t subservice);
	/**
	 * Retrieve SID from current packet
	 * @param sid [out]
	 * @return - @c RETURN_OK if SID retrieval successful
	 *         - @c RETURN_FAILED otherwise
	 */
	ReturnValue_t retrieveSID(uint32_t * sid);

	/* Subservice 1, 2 */
	ReturnValue_t handleAddingReportStructure();
	/* Subservice 3, 4 */
	ReturnValue_t handleDeletingReportStructure();
	ReturnValue_t deleteReportStructure(uint32_t sid, SidToStoreIdMap * sidMap);
	/* Subservice 5, 6, 7, 8 */
	ReturnValue_t switchPeriodicReportGeneration(bool enableGeneration);
	ReturnValue_t switchReportGeneration(uint32_t sid, SidToStoreIdMap * sidMap,
			bool enableGeneration);
	/* Subservice 9, 10, 11, 12 */
	ReturnValue_t handleStructureReporting();
	/* Subservice 27, 28 */
	ReturnValue_t generateOneParameterReport();
	/* Subservice 29, 30 */
	ReturnValue_t appendParametersToReport();
	void prepareNewData(uint32_t * updatedData, uint8_t * updatedNumberOfParams,
			HkPacketDefinition * oldHkPacket, hkAppendParametersCommand * newParams);
	/* Subservice 31, 32 */
	ReturnValue_t modifyCollectionInterval();

	/* Packet generation */
	/**
	 * Generate a HK packet with supplied SID and HK definition packet. Subservice 25, 26
	 * @param sid
	 * @param hkDefinition
	 * @return
	 */
	ReturnValue_t generateHkPacket(sid32_t sid, HkPacketDefinition * hkDefinition,
			uint8_t subservice);
	/**
	 * Send HK Buffer stored in Send Buffer. Called by generateHkPacket()
	 * @param sid
	 * @param hkDataSize
	 */
	void sendHkPacket(sid32_t sid, uint32_t hkDataSize,
			bool isDiagnostics, uint8_t subservice);

	//void poolIdSwapper(uint32_t * poolIdBuffer, uint32_t poolIdBufferLength);
};


#endif /* MISSION_PUS_SERVICE3HOUSEKEEPING_H_ */
