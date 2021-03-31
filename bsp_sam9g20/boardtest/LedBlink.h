#ifndef LED_Blink_H_
#define LED_Blink_H_


#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/objectmanager/SystemObject.h>

class LED_Blink : public SystemObject, public ExecutableObjectIF {
public:
	LED_Blink(const char * printName, object_id_t objectId);
	virtual ~LED_Blink();
	virtual ReturnValue_t performOperation(uint8_t operationCode = 0);

	const char * printName;
};

#endif /* LED_Blink_H_ */
