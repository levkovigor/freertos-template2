#ifndef SAM9G20_TMTCBRIDGE_TMTCLWIPUDPBRIDGE_H_
#define SAM9G20_TMTCBRIDGE_TMTCLWIPUDPBRIDGE_H_

 /* UDP local connection port */
 #define UDP_SERVER_PORT    7

 /* UDP remote connection port */
 #define UDP_CLIENT_PORT    2008

#include <fsfw/tmtcservices/TmTcBridge.h>

extern "C" {
#include <sam9g20/lwip/include/lwip/udp.h>
}

/**
 * @brief 	Handles TMTC reception via UDP, using the lightweight IP
 * 			stack (lwIP)
 * @author 	J. Meier, R. Mueller
 */
class TmTcLwIpUdpBridge : public TmTcBridge {
public:
	TmTcLwIpUdpBridge(object_id_t objectId_, object_id_t ccsdsPacketDistributor_);
	virtual ~TmTcLwIpUdpBridge();

	virtual ReturnValue_t initialize();

	/**
	 * Initialize UDP server
	 * @return
	 */
	ReturnValue_t udp_server_init();

	/**
	 * In addition to default implementation, ethernet link status is checked.
	 * @param operationCode
	 * @return
	 */
	virtual ReturnValue_t performOperation(uint8_t operationCode = 0);

	/** TM Send implementation uses udp_send function from lwIP stack
	 * @param data
	 * @param dataLen
	 * @return
	 */
	virtual ReturnValue_t sendTm(const uint8_t * data, uint32_t dataLen);

	/**
	 * TC Receive implementation empty, uses callback function from lwIP stack
	 * @return
	 */
	virtual ReturnValue_t receiveTc(uint8_t ** buffer, uint32_t * size);

	/**
	 * @brief This function is called when an UDP datagram has been received on the port UDP_PORT.
	 * @param arg
	 * @param upcb_
	 * @param p
	 * @param addr Source address which will be bound to TmTcUdpBridge::lastAdd
	 * @param port
	 */
	static void udp_server_receive_callback(void *arg, struct udp_pcb *upcb_, struct pbuf *p, const ip_addr_t *addr, u16_t port);
private:
	struct udp_pcb *upcb;
	ip_addr_t lastAdd;

	/**
	 * In addition to default Comm Link connect, display IP Address and Port of client
	 * @param ipAddress
	 * @param port
	 * @param p
	 */
	void registerClientConnect(uint32_t ipAddress,uint16_t port,struct pbuf* p);
};

#endif /* SAM9G20_TMTCUDPBRIDGE_TMTCUDPBRIDGE_H_ */
