#include <stm32/boardtest/UdpReceiveTask.h>

#include "lwip/timeouts.h"
#include "stm32h7xx_hal.h"

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>

extern "C" {
#include "bsp/utility/print.h"
#include "udp_echoserver.h"
#include "ethernetif.h"
#include "app_ethernet.h"
extern struct netif gnetif; /* network interface structure */
}
static uint32_t delay;
int i;

UdpReceiveTask::UdpReceiveTask(const char * printName, object_id_t objectId): SystemObject(objectId), printName(printName) {
	udp_echoserver_init();
}

UdpReceiveTask::~UdpReceiveTask() {

}

ReturnValue_t UdpReceiveTask::performOperation(uint8_t operationCode) {

    i=100;
	delay = HAL_GetTick();
    while(HAL_GetTick()<delay+1000){
    	while(HAL_GetTick() < delay+i){
    	}
    	i=i+100;
		/* Read a received packet from the Ethernet buffers and send it
		to the lwIP for handling */
		ethernetif_input(&gnetif);

		/* Handle timeouts */
		sys_check_timeouts();

		#if LWIP_NETIF_LINK_CALLBACK
			Ethernet_Link_Periodic_Handle(&gnetif);
		#endif

		#if LWIP_DHCP
			DHCP_Periodic_Handle(&gnetif);
		#endif
    }
	return HasReturnvaluesIF::RETURN_OK;
}
