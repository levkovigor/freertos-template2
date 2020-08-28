///**
// * @file ThermalMessage.h
// *
// * @date 16.02.2020
// */
//
//#ifndef CONFIG_IPC_THERMALMESSAGE_H_
//#define CONFIG_IPC_THERMALMESSAGE_H_
//
//#include<stdint.h>
//#include <fsfw/devicehandlers/CommunicationMessage.h>
//#include <config/ipc/MissionMessageTypes.h>
//
//typedef uint16_t rtd_t;
//
///**
// * @brief This message is used for the communication between the SPI PollingTask
// *        and the SPI Communication Interface
// */
//class SpiMessage: public CommunicationMessage {
//
//public:
//	SpiMessage();
//	virtual ~SpiMessage();
//
//	/**
//	 * Custom SPI Message Types can be added here
//	 */
//	enum messageId {
//		SPI_RTD = MESSAGE_TYPE::SPI_RTD
//	};
//
//	static void setRtdValue(CommunicationMessage *message, rtd_t rtdValue);
//	static rtd_t getRtdValue(const CommunicationMessage * message);
//
//	static void clear(CommunicationMessage* message);
//private:
//};
//
//#endif /* CONFIG_IPC_THERMALMESSAGE_H_ */
