#ifndef UDPRECEIVETASK_H_
#define UDPRECEIVETASK_H_

#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/objectmanager/SystemObject.h>

class UdpReceiveTask : public SystemObject, public ExecutableObjectIF {
public:
	UdpReceiveTask(const char * printName, object_id_t objectId);
	virtual ~UdpReceiveTask();

	virtual ReturnValue_t performOperation(uint8_t operationCode = 0);
	const char * printName;
};

#endif /* UDPRECEIVETASK_H_ */
