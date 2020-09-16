#ifndef MISSION_UTILITY_TMFUNNEL_H_
#define MISSION_UTILITY_TMFUNNEL_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/tmtcservices/AcceptsTelemetryIF.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/tmtcservices/TmTcMessage.h>

namespace Factory{
void setStaticFrameworkObjectIds();
}

/**
 * @brief       TM Recipient.
 * @details
 * Main telemetry receiver. All generated telemetry is funneled into
 * this object.
 * @ingroup     utility
 * @author      J. Meier
 */
class TmFunnel: public AcceptsTelemetryIF,
		public ExecutableObjectIF,
		public SystemObject {
	friend void (Factory::setStaticFrameworkObjectIds)();
public:
	TmFunnel(object_id_t objectId, uint32_t messageDepth = 20);
	virtual ~TmFunnel();

	virtual MessageQueueId_t getReportReceptionQueue(
			uint8_t virtualChannel = 0) override;
	virtual ReturnValue_t performOperation(uint8_t operationCode = 0) override;
	virtual ReturnValue_t initialize() override;

protected:
	static object_id_t downlinkDestination;
	static object_id_t storageDestination;

private:
	uint16_t sourceSequenceCount = 0;
	MessageQueueIF* tmQueue = nullptr;
	MessageQueueIF* storageQueue = nullptr;

	StorageManagerIF* tmPool = nullptr;
	uint32_t messageDepth = 0;

	ReturnValue_t handlePacket(TmTcMessage* message);
};

#endif /* MISSION_UTILITY_TMFUNNEL_H_ */
