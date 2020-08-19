#ifndef _UART0TESTTASK_H_
#define _UART0TESTTASK_H_

#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/objectmanager/SystemObjectIF.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <config/objects/systemObjectList.h>

extern "C" {
	#include "board.h"
	#include "AT91SAM9G20.h"
	#include <hal/Drivers/UART.h>
}

class UART0TestTask:
		public SystemObject,
		public ExecutableObjectIF,
		public HasReturnvaluesIF
{
public:
	UART0TestTask(const char * printName, object_id_t objectId);
	virtual ~UART0TestTask();


	virtual ReturnValue_t performOperation(uint8_t operationCode = 0);
	const char * printName;
private:

	UARTconfig configBus0 = { .mode = AT91C_US_USMODE_NORMAL
			| AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE
			| AT91C_US_OVER_16 | AT91C_US_NBSTOP_1_BIT, .baudrate = 115200,
			.timeGuard = 1, .busType = rs232_uart, .rxtimeout = 0xFFFF };
	int retValInt = 0;
	unsigned int readSize = 4, i;
	unsigned char readData[16] = { 0 }, writeData[16] = { 0 };
};

#endif /* _UART0TESTTASK_H_ */
