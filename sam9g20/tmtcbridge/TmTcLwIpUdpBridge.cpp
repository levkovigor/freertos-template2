#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/serialize/EndianSwapper.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/tmtcservices/AcceptsTelecommandsIF.h>
#include <sam9g20/tmtcbridge/TmTcLwIpUdpBridge.h>

extern "C" {
#include <sam9g20/at91/include/at91/boards/at91sam9g20-ek/ethernet/lwip_init.h>
}

TmTcLwIpUdpBridge::TmTcLwIpUdpBridge(object_id_t objectId_,
		object_id_t ccsdsPacketDistributor_):
			TmTcBridge(objectId_, ccsdsPacketDistributor_), upcb(nullptr) {
	TmTcLwIpUdpBridge::lastAdd.addr = IPADDR_TYPE_ANY;
}

TmTcLwIpUdpBridge::~TmTcLwIpUdpBridge() {

}

ReturnValue_t TmTcLwIpUdpBridge::initialize() {
	TmTcBridge::initialize();
	ReturnValue_t result = udp_server_init();
	return result;
}

ReturnValue_t TmTcLwIpUdpBridge::udp_server_init(void) {
	err_t err;
	/* Create a new UDP control block  */
	TmTcLwIpUdpBridge::upcb = udp_new();
	if (TmTcLwIpUdpBridge::upcb)
	{
		/* Bind the upcb to the UDP_PORT port */
		/* Using IP_ADDR_ANY allow the upcb to be used by any local interface */
		err = udp_bind(TmTcLwIpUdpBridge::upcb, IP_ADDR_ANY, UDP_SERVER_PORT);

		if(err == ERR_OK) // @suppress("Ambiguous problem")
		{
			/* Set a receive callback for the upcb */
			udp_recv(TmTcLwIpUdpBridge::upcb, &udp_server_receive_callback,(void*) this);
			return RETURN_OK;
		}
		else
		{
			udp_remove(TmTcLwIpUdpBridge::upcb);
			return RETURN_FAILED;
		}
	} else {
		return RETURN_FAILED;
	}
}

ReturnValue_t TmTcLwIpUdpBridge::performOperation(uint8_t operationCode) {
	// In case ethernet cable is disconnected
	if(not ethernetCableConnected and communicationLinkUp) {
		communicationLinkUp = false;
	}
	TmTcBridge::performOperation();
	return RETURN_OK;
}

ReturnValue_t TmTcLwIpUdpBridge::receiveTc(uint8_t ** buffer, uint32_t * size) {
	*size = 0;
	return RETURN_OK;
}

ReturnValue_t TmTcLwIpUdpBridge::sendTm(const uint8_t* data, uint32_t dataLen) {
	struct pbuf *p_tx = pbuf_alloc(PBUF_TRANSPORT, dataLen, PBUF_RAM); // @suppress("Invalid arguments")
	if ((p_tx != NULL) && (lastAdd.addr != IPADDR_TYPE_ANY) && (upcb != NULL)) {
		/* copy data to pbuf */
		err_t err  = pbuf_take(p_tx, (char*) data, dataLen);
		if(err!=ERR_OK){
			pbuf_free(p_tx);
			return err;
		}
		/* Connect to the remote client */
		err = udp_connect(TmTcLwIpUdpBridge::upcb, &lastAdd, UDP_CLIENT_PORT);
		if(err != ERR_OK){
			pbuf_free(p_tx);
			return err;
		}
		/* Tell the client that we have accepted it */
		err = udp_send(TmTcLwIpUdpBridge::upcb, p_tx);
		pbuf_free(p_tx);
		if(err!=ERR_OK){
			/* free the UDP connection, so we can accept new clients */
			udp_disconnect (TmTcLwIpUdpBridge::upcb);
			return err;
		}

		/* free the UDP connection, so we can accept new clients */
		udp_disconnect (TmTcLwIpUdpBridge::upcb);
	}else{
		return RETURN_FAILED;
	}
	return RETURN_OK;
}

void TmTcLwIpUdpBridge::udp_server_receive_callback(void* arg,
		struct udp_pcb* upcb_, struct pbuf* p, const ip_addr_t* addr,
		u16_t port) {
	struct pbuf *p_tx;
	TmTcLwIpUdpBridge * udpBridge = (TmTcLwIpUdpBridge *) arg;
	/* allocate pbuf from RAM*/
	p_tx = pbuf_alloc(PBUF_TRANSPORT,p->len, PBUF_RAM); // @suppress("Invalid arguments")

	if(p_tx != NULL)
	{
		udpBridge->upcb = upcb_;
		udpBridge->lastAdd = *addr;

		if(udpBridge->communicationLinkUp == false) {
			uint32_t ipAddress = addr->addr;
			udpBridge->registerClientConnect(ipAddress, port, p);
		}

		// this is an empty bytearray.
		// Used currently to initiate connection between udp server (dev board) and client (pc / raspberrypi...)
		// until a cleaner solution is found
		if(p->len == 0) {
			//info << "UDP Server: Empty bytearray received. No data processing" << std::endl;
			return;
		}

		pbuf_take(p_tx, (char*)p->payload, p->len);
		/* send the received data to the uart port */
		char* data = reinterpret_cast<char*>(p_tx->payload);
		*(data+p_tx->len) = '\0';

		// bytearray with five entries 0 means disconnected client for now
		if(p->len == 5 && data[0] == 0) {
			udpBridge->registerCommDisconnect();
			return;
		}

		//info << "UDP Server: Received data" << std::endl;
		//udpBridge->printData(p,data);

		store_address_t storeId;
		ReturnValue_t returnValue = udpBridge->tcStore->addData(&storeId,reinterpret_cast<uint8_t*>(p->payload), p->len);
		if (returnValue != RETURN_OK) {
			sif::debug << "UDP Server: Data storage failed" << std::endl;
			pbuf_free(p_tx);
			return;
		}
		TmTcMessage message(storeId);
		if (udpBridge->tmTcReceptionQueue->sendToDefault(&message) != RETURN_OK) {
			sif::debug << "UDP Server: Sending message to queue failed" << std::endl;
			udpBridge->tcStore->deleteData(storeId);
		}
	}
	/* Free the p_tx buffer */
	pbuf_free(p_tx);
}

void TmTcLwIpUdpBridge::registerClientConnect(uint32_t ipAddress, uint16_t port,struct pbuf* p) {
	TmTcBridge::registerCommConnect();
	int ipAddress1 = (ipAddress & 0xFF000000) >> 24;
	int ipAddress2 = (ipAddress & 0xFF0000) >> 16;
	int ipAddress3 = (ipAddress & 0xFF00) >> 8;
	int ipAddress4 = ipAddress & 0xFF;
	info << "UDP Server: Client IP Address " << std::dec << ipAddress4 << "." << ipAddress3 << "." << ipAddress2 << "." << ipAddress1 << std::endl;
	uint16_t portSwapped = EndianSwapper::swap(port);
	info << "UDP Server: Client IP Port " << (int)portSwapped << std::endl;
}
