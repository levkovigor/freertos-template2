#ifndef SAM9G20_BOARDTEST_SPITESTTASK_H_
#define SAM9G20_BOARDTEST_SPITESTTASK_H_

#include "OBSWConfig.h"

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <bsp_sam9g20/common/At91SpiDriver.h>

extern "C" {
#include <hal/Drivers/SPI.h>
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
		MGM_LIS3,
		AT91_LIB_BLOCKING,
		AT91_LIB_DMA,
		IOBC_FRAM
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
	unsigned char readData[1024] = {0};
	unsigned char writeData[1024] = {0};
	SPIslaveParameters slaveParams= {};
	SPItransfer spiTransfer = {};
	bool initWait = true;
    bool oneshot = true;
    int utilityCounter = 0;

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
	void performAt91LibTest();

	SPIbus SPI_bus = bus1_spi; //!< SPI bus 0 can not be used on iOBC!
	static void spiIrqHandler(At91SpiBuses bus, At91TransferStates state, void* args);

#ifdef ISIS_OBC_G20
    static volatile At91TransferStates transferState;
    void iobcFramTestBlocking();
	void iobcFramTestInterrupt();
	void iobcFramRawTest();
	static void spiCallback(At91SpiBuses bus, At91TransferStates state, void* args);
#endif

};





#endif /* SAM9G20_BOARDTEST_SPITESTTASK_H_ */
