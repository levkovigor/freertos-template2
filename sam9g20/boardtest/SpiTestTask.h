#ifndef SAM9G20_BOARDTEST_SPITESTTASK_H_
#define SAM9G20_BOARDTEST_SPITESTTASK_H_
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>

extern "C" {
#include "SPI.h"
}

class SpiTestTask: public SystemObject,
	public ExecutableObjectIF,
	public HasReturnvaluesIF
{
public:
	enum SpiTestMode: uint8_t {
		BLOCKING,
		NON_BLOCKING,
		PT1000,
		GYRO,
		MGM_LIS3
	};

	SpiTestTask(object_id_t objectId, SpiTestMode spiTestMode);
	virtual ~SpiTestTask();

	virtual ReturnValue_t performOperation(uint8_t operationCode = 0);

private:

	/**
	 * Testing SPI_writeRead(), blocking function
	 */
	void performBlockingSpiTest(SPIslave slave, uint8_t bufferPosition);
	void performNonBlockingSpiTest(SPIslave slave, uint8_t bufferPosition);

	static void SPIcallback(SystemContext context, xSemaphoreHandle semaphore);

	SpiTestMode spiTestMode = SpiTestMode::PT1000;
	uint8_t transferSize = 3;
	unsigned char readData[128] = {0};
	unsigned char writeData[128] = {0};
	SPIslaveParameters slaveParams;
	SPItransfer spiTransfer;
	bool initWait = true;

	bool decoderSSused = false;
	void configureSpiDummySSIfNeeded();

	void performBlockingPt1000Test();

	void performBlockingGyroTest();
	bool gyroConfigured = false;
	static constexpr uint8_t GYRO_READ_MASK = 0x80;
	static constexpr uint8_t GYRO_WRITE_MASK = 0x7F;

	void writeGyroRegister(SPItransfer& spiTransfer, uint8_t reg, uint8_t value);
	uint8_t readGyroRegister(SPItransfer& spiTransfer, uint8_t reg);

	void configureGyroPmu(SPItransfer& spiTransfer);
	uint8_t readGyroPmu(SPItransfer& spiTransfer);
	void readGyroSensorsSeparateReads(SPItransfer& spiTransfer);
	void readGyroSensorsBlockRead(SPItransfer& spiTransfer);

	void performBlockingMgmTest();

	SPIbus SPI_bus = bus1_spi; //!< SPI bus 0 can not be used on iOBC!
};





#endif /* SAM9G20_BOARDTEST_SPITESTTASK_H_ */
