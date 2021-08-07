#include <fsfw/osal/freertos/MessageQueue.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <bsp_sam9g20/tmtcbridge/EmacPollingTask.h>

#define BOARD_EMAC_PHY_COMP_DM9161 1

extern "C" {
#include <at91/boards/at91sam9g20-ek/ethernet/emacif.h>
#include <at91/boards/at91sam9g20-ek/drivers/macb/macb.h>
#include <at91/boards/at91sam9g20-ek/ethernet/lwip_init.h>

extern struct netif *netif;  /* network interface structure */
extern Macb mac;
}

EmacPollingTask::EmacPollingTask(object_id_t objectId_): // @suppress("Class members should be properly initialized")
	SystemObject(objectId_), connectorCheckCounter(0), disconnectCheckCounter(0) {
}

EmacPollingTask::~EmacPollingTask() {
}

ReturnValue_t EmacPollingTask::initialize() {
	return RETURN_OK;
}


/* Poll the EMAC Interface and pass content to the network interface (lwIP) */
ReturnValue_t EmacPollingTask::performOperation(uint8_t operationCode) {
	// global trace parameter,is set to 3 temporarily to prevent overload of messages
	checkEthernetConnection();
	if(ethernetCableConnected) {
		// this function appears to take a long time because of the timeout
		// in the AutoNegotiateFunction
		// commented out for now !!
		// as long as the AutoNegotiateFunction is using a permanent loop
		// with a max retry counter we can't use this function
		// checkForDisconnect();
	}
	setTrace(3);
	/* read the emac interface */
	emacif_poll(netif);
	setTrace(5);
	return RETURN_OK;
}


void EmacPollingTask::checkEthernetConnection() {
	if(ethernetCableConnected == false ) {
		connectorCheckCounter++;
	}
	if(ethernetCableConnected == false &&
			connectorCheckCounter == CONNECTOR_CHECK_TRIGGER) {
		/* Auto Negotiate */
		Macb* pMac = &mac;
		// indexer can't resolve because it's in an #if clause
		setTrace(3);
 		ReturnValue_t result = MACB_AutoNegotiate(pMac);
 		setTrace(5);
		if(result == 0){
			sif::info << "UDP Server: Ethernet Cable not connected" << std::endl;
		} else {
			sif::info << "UDP Server: Ethernet Cable connected" << std::endl;
			ethernetCableConnected = true;
		}
		connectorCheckCounter = 0;
	}
}

void EmacPollingTask::checkForDisconnect() {
	if(ethernetCableConnected) {
		disconnectCheckCounter ++;
	}
	if(disconnectCheckCounter == DISCONNECT_CHECK_TRIGGER) {
		/* Auto Negotiate */
		Macb* pMac = &mac;
		traceLevel = 3; // @suppress("Symbol is not resolved")
		ReturnValue_t result = MACB_AutoNegotiate(pMac);
		traceLevel = 5; // @suppress("Symbol is not resolved")
		if(result == 0) {
			sif::info << "UDP Server: Ethernet "
					"Cable not connected anymore !" << std::endl;
			ethernetCableConnected = false;
			connectorCheckCounter = 0;
		}
		disconnectCheckCounter = 0;
	}
}
