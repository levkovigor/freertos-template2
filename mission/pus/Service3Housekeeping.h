#ifndef MISSION_PUS_SERVICE3HOUSEKEEPINGSERVICE_H_
#define MISSION_PUS_SERVICE3HOUSEKEEPINGSERVICE_H_
#include <fsfw/housekeeping/AcceptsHkPacketsIF.h>
#include <fsfw/housekeeping/HousekeepingMessage.h>
#include <fsfw/tmtcservices/CommandingServiceBase.h>

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
 * This is the CommandingServiceBase implementation which uses local data pools.
 * Pre-defined HK packets are added in config/objects/factory.
 * @author 	R. Mueller
 * @ingroup pus_services
 */
class Service3Housekeeping: public CommandingServiceBase,
		public AcceptsHkPacketsIF {
public:
	static constexpr uint8_t NUM_OF_PARALLEL_COMMANDS = 4;
	static constexpr uint16_t COMMAND_TIMEOUT_SECONDS = 60;

	Service3Housekeeping(object_id_t objectId, uint16_t apid, uint8_t serviceId);
	virtual~ Service3Housekeeping();
protected:
	/* CSB abstract functions implementation . See CSB documentation. */
	ReturnValue_t isValidSubservice(uint8_t subservice) override;
	ReturnValue_t getMessageQueueAndObject(uint8_t subservice,
			const uint8_t *tcData, size_t tcDataLen, MessageQueueId_t *id,
			object_id_t *objectId) override;
	ReturnValue_t prepareCommand(CommandMessage* message,
			uint8_t subservice, const uint8_t *tcData, size_t tcDataLen,
			uint32_t *state, object_id_t objectId) override;
	ReturnValue_t handleReply(const CommandMessage* reply,
			Command_t previousCommand, uint32_t *state,
			CommandMessage* optionalNextCommand, object_id_t objectId,
			bool *isStep) override;

	virtual MessageQueueId_t getHkQueue() const;
private:
	enum class Subservice {
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
	};

	ReturnValue_t checkAndAcquireTargetID(object_id_t* objectIdToSet,
			const uint8_t* tcData, size_t tcDataLen);
	ReturnValue_t checkInterfaceAndAcquireMessageQueue(
			MessageQueueId_t* messageQueueToSet, object_id_t* objectId);

	ReturnValue_t generateHkReport(const CommandMessage* hkMessage);
	void handleUnrequestedReply(CommandMessage* reply) override;
};



#endif /* MISSION_PUS_SERVICE3HOUSEKEEPINGSERVICE_H_ */
