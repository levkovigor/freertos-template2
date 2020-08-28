///**
// * @file SPI_ComTask.cpp
// * @date 21.02.2020
// * @author R. Mueller
// */
//
//#ifndef MISC_ARCHIVE_SPIPOLLINGTASK_H_
//#define MISC_ARCHIVE_SPIPOLLINGTASK_H_
//
//#include <fsfw/objectmanager/SystemObject.h>
//#include <fsfw/tasks/ExecutableObjectIF.h>
//#include <fsfw/returnvalues/HasReturnvaluesIF.h>
//#include <fsfw/ipc/MessageQueueIF.h>
//#include <fsfw/devicehandlers/DeviceHandlerBase.h>
//#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>
//#include <misc/archive/SpiMessage.h>
//#include <unordered_map>
//#include <atomic>
//
//extern "C"{
//#include "SPI.h"
//}
//
///**
// * @brief This function calls the actual ISIS driver functions
// *        to poll all SPI device. It also handles the GPIO operations
// *        necessary to switch multiplexer lines.
// *
// * @details
// * All commands are issued by device handlers and relayed by the
// * SPI ComIF to this polling task.
// * Two tasks were used to avoid the complexity of a single task
// * and to separate this interrupt-heavy task from the ComIF which can also
// * be interrupted by the device handlers.
// *
// * Furthermore, separate receive buffers are used for maximum safety
// * in this first implementation.
// */
//class SpiPollingTask: public SystemObject,
//		public ExecutableObjectIF,
//		public HasReturnvaluesIF
//{
//public:
//
//	SpiPollingTask(object_id_t objectId_);
//	virtual ~ SpiPollingTask();
//
//	/**
//	 * Called periodically
//	 * @param operationCode
//	 * @return
//	 */
//	ReturnValue_t performOperation(uint8_t operationCode = 0);
//	/**
//	 * Called by object manager on mission initialization
//	 * @return
//	 */
//	ReturnValue_t initialize();
//
//	static const uint16_t SPI_BUFFER_SIZE = 1024;
//
//	static inline std::atomic_int higherTaskUnlockedCounter = 0;
//private:
//	/**
//	 * Reception queue for messages from the SPI_ComTask
//	 */
//	MessageQueueIF* communicationQueue;
//
//	/**
//	 * Handles the reception of SPI messages from the
//	 * SPI Communication Interface.
//	 */
//	void receiveSpiMessages();
//
//	/**
//	 * Performs the polling of all SPI devices
//	 */
//	void performPollingOperation();
//
//	/**
//	 * For each decoder output select, 4 queue transfers are initiated consecutively.
//	 * To to increase code readability, this struct is passed to the function performing that operation.
//	 */
//	struct SendRequestBatch {
//		bool * sendRequested1;
//		bool * sendRequested2;
//		bool * sendRequested3;
//		bool * sendRequested4;
//	};
//
//	/** Helper struct as communication is done in send request packets with 4 binary semaphores */
//	struct SemaphorePack {
//		BinarySemaphore * semaph1;
//		BinarySemaphore * semaph2;
//		BinarySemaphore * semaph3;
//		BinarySemaphore * semaph4;
//	};
//
//	void handleSendRequestBatch(SendRequestBatch * sendRequests, SemaphorePack semaphPack);
//	void handleTransfer(bool * sendRequestPointer,
//				BinarySemaphore * binSemaphore);
//	void checkTransferResults(SendRequestBatch * sendRequests, SemaphorePack semaphPack, bool areDecoderDevices = true);
//	void checkTransferResult(bool * sendRequestBool,
//			BinarySemaphore * binSemaph, bool isDecoderDevice = true);
//	/**
//	 * The data can be stored and relayed in different ways,
//	 * depending on implementation and data size and concrete implementation.
//	 *
//	 * Ideally, this task has a large buffer and only passes the pointer
//	 * to the buffer and the respective buffer position to the ComIF.
//	 * In the first implementation, we're going to send data either in raw mode
//	 * or in the IPC store.
//	 */
//	enum ComType {
//		RAW_RTD,      //!< RAW
//		IPC_STORE,//!< IPC_STORE
//		POINTER   //!< POINTER
//	};
//
//	/**
//	 * This struct contains important information
//	 * for the queue transfer.
//	 * It is used in conjuction with the send request booleans,
//	 * as the master initiates all transfers.
//	 */
//	struct SpiInfo {
//		address_t logicalAddress; //!< Physical address of device (object ID of Device Handler for SPI devices)
//		ComType storageType;  //!< Specify how the data is transfered to the ComIF
//		// One missed reply is allowed for now
//		uint8_t errorCounter; //!< Incremented on queueTransfer timeout or other issues. Sends fault message to SPI ComIF if too high
//		SPIslaveParameters * slaveParameter; //!< Pointer to respective slave parameter struct
//		const uint8_t * sendData; //!< Stores the pointer  to the data to be sent
//		uint16_t sendDataSize; //!< Stores the size of the data to be sent
//		uint16_t readBufferPosition; //!< If data is transferred raw, the ComIF stores the data at this position
//	};
//
//	/* This map will map the address of the device select booleans to
//	 * the SPI information structure
//	 */
//	typedef std::unordered_map<bool *, struct SpiInfo> SendRequestMap;
//	typedef std::pair<bool *,struct SpiInfo> SendRequestEntry;
//	typedef SendRequestMap::iterator SendRequestIter;
//	typedef std::pair<SendRequestIter,bool> SendRequestResult;
//
//	SendRequestMap sendRequestMap;
//
//	static const TickType_t SPI_STANDARD_SEMAPHORE_TIMEOUT = 10; //!< Default semaphore timout in ticks
//	static const uint8_t SPI_DELAY_CS_LOW_TO_CLOCK = 1; //!< Delay between CS-low to first clock transition. See AT91SAM9G20 datasheet.
//	static const uint8_t SPI_DELAY_CONSECUTIVE_BYTES = 2; //!< Delay between consecutive bytes to the same slave. See AT91SAM9G20 datasheet.
//	static const uint32_t  SPI_BUS_SPEED =
//			(SPI_MAX_BUS_SPEED + SPI_MIN_BUS_SPEED)/2;//!< The speed in Hertz of the SPI clock signal. Minimum SPI_MIN_BUS_SPEED and Maximum SPI_MAX_BUS_SPEED.
//	static const SPIbus SPI_BUS1 = bus1_spi; //!< Only bus 1 is used for external communication
//	/*!
//	 * For adding delays between successive transaction to the same slave.
//	 * The driver will wait the specified amount of ticks before it proceeds to process the next transfer in the queue.
//	 */
//	static const uint8_t SPI_POST_TRANSFER_DELAY = 0;
//
//	/**
//	 * SPI Slave parameters which will be similar for each transfer
//	 */
//	SPIslaveParameters slave0Params;
//	SPIslaveParameters slave1Params;
//	SPIslaveParameters slave2Params;
//	SPIslaveParameters slave3Params;
//	SPIslaveParameters slavePt1000Params;
//	/**
//	 * Contains device specific transfer properties:
//	 * writeBuffer, readBuffer pointers and callback.
//	 */
//	SPItransfer spiTransfer;
//
//	/**
//	 * A boolean to perform the send request.
//	 * The address of this boolean is used to map the respective devices and all
//	 * SPI transfer properties.
//	 * The SPI polling has to be run in a specific order because of the
//	 * demultiplexer logic. These booleans are used in performOperation()
//	 * to distinguish the order.
//	 */
//	bool SPI_SS0_SendRequest = false;
//	bool SPI_SS1_SendRequest = false;
//	bool SPI_SS2_SendRequest = false;
//	bool SPI_SS3_SendRequest = false;
//
//	bool SPI_SS4_Dec1_O1_SendRequest = false;
//	bool SPI_SS4_Dec1_O2_SendRequest = false;
//	bool SPI_SS4_Dec1_O3_SendRequest = false;
//	bool SPI_SS4_Dec1_O4_SendRequest = false;
//	bool SPI_SS4_Dec1_O5_SendRequest = false;
//	bool SPI_SS4_Dec1_O6_SendRequest = false;
//	bool SPI_SS4_Dec1_O7_SendRequest = false;
//	bool SPI_SS4_Dec1_O8_SendRequest = false;
//
//	bool SPI_SS5_Dec2_O1_SendRequest = false;
//	bool SPI_SS5_Dec2_O2_SendRequest = false;
//	bool SPI_SS5_Dec2_O3_SendRequest = false;
//	bool SPI_SS5_Dec2_O4_SendRequest = false;
//	bool SPI_SS5_Dec2_O5_SendRequest = false;
//	bool SPI_SS5_Dec2_O6_SendRequest = false;
//	bool SPI_SS5_Dec2_O7_SendRequest = false;
//	bool SPI_SS5_Dec2_O8_SendRequest = false;
//
//	bool SPI_SS6_Dec3_O1_SendRequest = false;
//	bool SPI_SS6_Dec3_O2_SendRequest = false;
//	bool SPI_SS6_Dec3_O3_SendRequest = false;
//	bool SPI_SS6_Dec3_O4_SendRequest = false;
//	bool SPI_SS6_Dec3_O5_SendRequest = false;
//	bool SPI_SS6_Dec3_O6_SendRequest = false;
//	bool SPI_SS6_Dec3_O7_SendRequest = false;
//	bool SPI_SS6_Dec3_O8_SendRequest = false;
//
//	bool SPI_SS7_Dec4_O1_SendRequest = false;
//	bool SPI_SS7_Dec4_O2_SendRequest = false;
//	bool SPI_SS7_Dec4_O3_SendRequest = false;
//	bool SPI_SS7_Dec4_O4_SendRequest = false;
//	bool SPI_SS7_Dec4_O5_SendRequest = false;
//	bool SPI_SS7_Dec4_O6_SendRequest = false;
//	bool SPI_SS7_Dec4_O7_SendRequest = false;
//	bool SPI_SS7_Dec4_O8_SendRequest = false;
//
//	SendRequestBatch sendRequestsSingleSlaveSelect = {&SPI_SS0_SendRequest, &SPI_SS1_SendRequest,
//			&SPI_SS2_SendRequest, &SPI_SS3_SendRequest};
//	SendRequestBatch sendRequestsDecOutput1 = {&SPI_SS4_Dec1_O1_SendRequest,
//			&SPI_SS5_Dec2_O1_SendRequest,&SPI_SS6_Dec3_O1_SendRequest,&SPI_SS7_Dec4_O1_SendRequest};
//	SendRequestBatch sendRequestsDecOutput2 = {&SPI_SS4_Dec1_O2_SendRequest,
//			&SPI_SS5_Dec2_O2_SendRequest,&SPI_SS6_Dec3_O2_SendRequest,&SPI_SS7_Dec4_O2_SendRequest};
//	SendRequestBatch sendRequestsDecOutput3 = {&SPI_SS4_Dec1_O3_SendRequest,
//			&SPI_SS5_Dec2_O3_SendRequest,&SPI_SS6_Dec3_O3_SendRequest,&SPI_SS7_Dec4_O3_SendRequest};
//	SendRequestBatch sendRequestsDecOutput4 = {&SPI_SS4_Dec1_O4_SendRequest,
//			&SPI_SS5_Dec2_O4_SendRequest,&SPI_SS6_Dec3_O4_SendRequest,&SPI_SS7_Dec4_O4_SendRequest};
//	SendRequestBatch sendRequestsDecOutput5 = {&SPI_SS4_Dec1_O5_SendRequest,
//			&SPI_SS5_Dec2_O5_SendRequest,&SPI_SS6_Dec3_O5_SendRequest,&SPI_SS7_Dec4_O5_SendRequest};
//	SendRequestBatch sendRequestsDecOutput6 = {&SPI_SS4_Dec1_O6_SendRequest,
//			&SPI_SS5_Dec2_O6_SendRequest,&SPI_SS6_Dec3_O6_SendRequest,&SPI_SS7_Dec4_O6_SendRequest};
//	SendRequestBatch sendRequestsDecOutput7 = {&SPI_SS4_Dec1_O7_SendRequest,
//			&SPI_SS5_Dec2_O7_SendRequest,&SPI_SS6_Dec3_O7_SendRequest,&SPI_SS7_Dec4_O7_SendRequest};
//	SendRequestBatch sendRequestsDecOutput8 = {&SPI_SS4_Dec1_O8_SendRequest,
//			&SPI_SS5_Dec2_O8_SendRequest,&SPI_SS6_Dec3_O8_SendRequest,&SPI_SS7_Dec4_O8_SendRequest};
//
//	uint8_t readBuffer [SPI_BUFFER_SIZE] = {0};
//
//	void prepareSpiTransfer(SPItransfer * spiTransfer, SpiInfo * spiInfo,
//				BinarySemaphore * binSemaphore, SPIslaveParameters * slaveParams);
//	static void SPIcallback(SystemContext context, xSemaphoreHandle semaphore);
//	void prepareSpiRtdMessage(SpiMessage * spiMessage, SpiInfo * spiInfo);
//};
//
//#endif /* MISC_ARCHIVE_SPIPOLLINGTASK_H_ */
