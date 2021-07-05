#ifndef MISSION_TWITESTTASK_H_
#define MISSION_TWITESTTASK_H_

#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>
extern "C" {
#include <hal/Drivers/I2C.h>
}

#include <vector>
#include <string>
#include <cmath>


class TwiTestTask: public SystemObject, public ExecutableObjectIF, public HasReturnvaluesIF {
public:
	enum twiModes: uint8_t {
		ECHO,
		ECHO_NON_BLOCKING,
		GPS
	};

	enum comStates: uint8_t {
		WRITE,
		WAIT_WRITE,
		READ,
		WAIT_READ
	};

	enum readingModes: uint8_t {
		BYTE_FOR_BYTE,
		FRAMES
	};

	TwiTestTask(object_id_t objectId, uint8_t slaveAddress);
	virtual ~TwiTestTask();

	virtual ReturnValue_t performOperation(uint8_t operationCode = 0);
private:
	static const uint32_t BUS_SPEED_HZ = 40000;
	static const uint8_t I2C_TIMEOUT = 5;
	static const uint8_t MAX_GPS_PACKET_SIZE = 66;
	// maximum size of I2C frame
	static const uint8_t MAX_FRAME_SIZE = 32;
	static const uint8_t FRAME_READ_COUNT = ceil((double)MAX_GPS_PACKET_SIZE/(double)MAX_FRAME_SIZE);
	static const uint8_t LAST_READ_BYTE_NUMBER = MAX_GPS_PACKET_SIZE % MAX_FRAME_SIZE;

	I2CgenericTransfer i2cTransfer;
	I2CtransferStatus transferResult = I2CtransferStatus::done_i2c;
	BinarySemaphore binSemaph;

	uint8_t slaveAddress = 0;
	std::string testString = "Hello, this is a message!";
	size_t testSize = testString.size();
	std::vector<uint8_t> CommandBuffer;
	std::vector<uint8_t> ReplyBuffer;
	uint8_t gpsCommandSize = 0;
	bool testCommandSent = false;
	uint8_t cycleCount = 1;

	uint32_t startTime = 0;
	uint32_t endTime = 0;
	uint8_t sendTrigger = 0;
	uint8_t printTrigger = 0;

	void performEchoOperation();
	void performI2cTestWrite();
	void performI2cTestRead();

	void performGpsOperation();
	void performI2cGpsWrite();
	void performI2cGpsRead();

	void performNonBlockingEcho();
	void performNonBlockingWrite();
	void performNonBlockingRead();
	void checkComStatus();

	/**
	 * Sends a read request for each byte. This is rather inefficient when using DMA.
	 * Frame reading is preferred for now.
	 * @param buffer
	 */
	void readI2cByteWise(uint8_t * buffer);
	/**
	 * Arduino is interrupt driven. To read more than the MAX_FRAME_SIZE,
	 * multiple read requests must be sent. This is performed by this function
	 * which sends read requests until the full GPS packet has been received.
	 * @param buffer
	 */
	void readI2cAsFrames(uint8_t * buffer);
	void interpretGpsPacket(uint8_t * gpsBuffer);

	void printBufferAsHex(uint8_t * buffer, char endMarker, uint8_t maxBufferSize);

	void prepareGpsConfigTest(uint8_t gpsFrequency_);

	twiModes twiMode;
	comStates comState;
	readingModes readingMode;

	void initializeI2cTransferStruct();
	static void I2cCallback(SystemContext context, xSemaphoreHandle semaphore);
};

#endif /* MISSION_TWITESTTASK_H_ */
