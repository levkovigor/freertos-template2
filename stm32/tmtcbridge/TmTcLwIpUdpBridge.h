/**
 * @file TmTcUdpBridge.h
 *
 * @date 13.02.2019
 * @author J. Meier, R. Mueller
 */

#ifndef BSP_TMTCUDPBRIDGE_TMTCUDPBRIDGE_H_
#define BSP_TMTCUDPBRIDGE_TMTCUDPBRIDGE_H_

#include <fsfw/tmtcservices/TmTcBridge.h>

extern "C" {
#include <stm32/lwip/udp.h>
}

class TmTcLwIpUdpBridge : public TmTcBridge{
public:
	TmTcLwIpUdpBridge(object_id_t objectId_, object_id_t ccsdsPacketDistributor_);
	virtual ~TmTcLwIpUdpBridge();

	virtual ReturnValue_t initialize();
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
	virtual ReturnValue_t sendTm(const uint8_t * data, size_t dataLen);

	/**
	 * TC Receive implementation empty, uses callback function from lwIP stack
	 * @return
	 */
	virtual ReturnValue_t receiveTc(uint8_t ** recvBuffer, size_t * size);

	/**
	 * @brief This function is called when an UDP datagram has been received on the port UDP_PORT.
	 * @param arg
	 * @param upcb_
	 * @param p
	 * @param addr Source address which will be bound to TmTcUdpBridge::lastAdd
	 * @param port
	 */
	static void udp_server_receive_callback(void *arg, struct udp_pcb *upcb_,
			struct pbuf *p, const ip_addr_t *addr, u16_t port);

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

#endif /* TMTCUDPBRIDGE_TMTCUDPBRIDGE_H_ */
