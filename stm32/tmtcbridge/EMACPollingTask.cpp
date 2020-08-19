#include <stm32/tmtcbridge/EMACPollingTask.h>

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <stm32/lwip/timeouts.h>

extern "C" {
#include <stm32/Inc/ethernetif.h>
#include <stm32/Inc/app_ethernet.h>
extern struct netif gnetif; /* network interface structure */
}

EmacPollingTask::EmacPollingTask(object_id_t objectId_): // @suppress("Class members should be properly initialized")
		SystemObject(objectId_),periodicHandleCounter(0) {
}

EmacPollingTask::~EmacPollingTask() {
}

ReturnValue_t EmacPollingTask::initialize() {
	return RETURN_OK;
}


/* Poll the EMAC Interface and pass content to the network interface (lwIP) */
ReturnValue_t EmacPollingTask::performOperation(uint8_t operationCode) {
	/* Read a received packet from the Ethernet buffers and send it
	 * to the lwIP for handling
	 */
	ethernetif_input(&gnetif);

	periodicHandleCounter ++;
	if (periodicHandleCounter == PERIODIC_HANDLE_TRIGGER) {
	#if LWIP_NETIF_LINK_CALLBACK
		Ethernet_Link_Periodic_Handle(&gnetif);
	#endif

	#if LWIP_DHCP
		DHCP_Periodic_Handle(&gnetif);
	#endif
	periodicHandleCounter = 0;
	}
	return RETURN_OK;
}
