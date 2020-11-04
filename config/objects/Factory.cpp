#include "Factory.h"

/* Config */
#include "../OBSWConfig.h"
#include "../tmtc/apid.h"
#include "../objects/systemObjectList.h"
#include "../devices/logicalAddresses.h"
#include "../devices/powerSwitcherList.h"
#include "../tmtc/pusIds.h"


/* Flight Software Framework */
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/internalError/InternalErrorReporter.h>
#include <fsfw/storagemanager/PoolManager.h>
#include <fsfw/tcdistribution/CCSDSDistributor.h>
#include <fsfw/tcdistribution/PUSDistributor.h>
#include <fsfw/events/EventManager.h>
#include <fsfw/fdir/FailureIsolationBase.h>
#include <fsfw/health/HealthTable.h>
#include <fsfw/pus/CService200ModeCommanding.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>

/* PUS Includes */
#include <fsfw/tmtcservices/PusServiceBase.h>
#include <fsfw/pus/Service1TelecommandVerification.h>
#include <fsfw/pus/Service2DeviceAccess.h>
#include <fsfw/pus/CService201HealthCommanding.h>
#include <fsfw/pus/Service3Housekeeping.h>
#include <fsfw/pus/Service5EventReporting.h>
#include <fsfw/pus/Service8FunctionManagement.h>
#include <fsfw/timemanager/TimeStamper.h>
#include <mission/controller/ThermalController.h>
#include <mission/pus/Service6MemoryManagement.h>
#include <mission/pus/Service17CustomTest.h>
#include <mission/pus/Service20ParameterManagement.h>
#include <mission/pus/Service23FileManagement.h>
#include <mission/utility/TmFunnel.h>
#include <mission/devices/PCDUHandler.h>
#include <mission/devices/GPSHandler.h>
#include <mission/devices/ThermalSensorHandler.h>
#include <mission/devices/GyroHandler.h>
#include <mission/fdir/PCDUFailureIsolation.h>

/* Test files */
#include <test/testinterfaces/DummyEchoComIF.h>
#include <test/testinterfaces/DummyGPSComIF.h>
#include <test/testinterfaces/DummyCookie.h>
#include <test/testdevices/ArduinoDeviceHandler.h>

/* Board Support Package Files */
#include <sam9g20/boardtest/AtmelArduinoHandler.h>
#include <sam9g20/boardtest/AtmelTestTask.h>
#include <sam9g20/tmtcbridge/TmTcSerialBridge.h>
#include <sam9g20/boardtest/TwiTestTask.h>
#include <sam9g20/boardtest/UART0TestTask.h>
#include <sam9g20/boardtest/UART2TestTask.h>
#include <sam9g20/boardtest/SpiTestTask.h>
#include <sam9g20/comIF/I2cDeviceComIF.h>
#include <sam9g20/comIF/GpioDeviceComIF.h>
#include <sam9g20/comIF/RS232DeviceComIF.h>
#include <sam9g20/comIF/SpiDeviceComIF.h>
#include <sam9g20/comIF/RS232PollingTask.h>
#include <sam9g20/core/CoreController.h>
#include <sam9g20/core/SoftwareImageHandler.h>
#include <sam9g20/core/SystemStateTask.h>
#include <sam9g20/memory/FRAMHandler.h>
#include <sam9g20/memory/SDCardHandler.h>
#include <sam9g20/pus/Service9CustomTimeManagement.h>
#include <sam9g20/boardtest/LedTask.h>

#include <test/testdevices/TestDeviceHandler.h>

#if defined(ETHERNET)
#include <sam9g20/tmtcbridge/EmacPollingTask.h>
#include <sam9g20/tmtcbridge/TmTcUdpBridge.h>
#endif


#include <cstdint>


/**
 * Build tasks by using SystemObject Interface (Interface).
 * Header files of all tasks must be included
 * Please note that an object has to implement the system object interface
 * if the nterface validity is checked or retrieved later by using the
 * get<TargetInterface>(object_id) function from the ObjectManagerIF.
 *
 * Framework objects are created first.
 *
 * @ingroup init
 */
void Factory::produce(void) {
	setStaticFrameworkObjectIds();
	new EventManager(objects::EVENT_MANAGER);
	new HealthTable(objects::HEALTH_TABLE);
	new InternalErrorReporter(objects::INTERNAL_ERROR_REPORTER);

	/* Pool manager handles storage und mutexes */
	{
	    // TODO: make this configurable via OBSWConfig.h
	    LocalPool::LocalPoolConfig poolConfig = {
	            {250, 32}, {120, 64}, {100, 128}, {50, 256},
	            {25, 512}, {10, 1024}, {10, 2048}
	    };
	    PoolManager* ipcStore = new PoolManager(objects::IPC_STORE, poolConfig);
	    size_t additionalSize = 0;
	    size_t storeSize = ipcStore->getTotalSize(&additionalSize);
        sif::info << "Allocating " << storeSize + additionalSize << " bytes "
                <<"for the IPC Store.." << std::endl;
	}

	{
        LocalPool::LocalPoolConfig poolConfig = {
                {500, 32}, {250, 64}, {120, 128}, {60, 256},
                {30, 512}, {15, 1024}, {10, 2048}
        };
        PoolManager* tmStore = new PoolManager(objects::TM_STORE, poolConfig);
        size_t additionalSize = 0;
        size_t storeSize = tmStore->getTotalSize(&additionalSize);
        sif::info << "Allocating " << storeSize + additionalSize << " bytes "
                <<"for the TM Store.." << std::endl;
	}

	{
        LocalPool::LocalPoolConfig poolConfig = {
                {500, 32}, {250, 64}, {120, 128},
                {60, 256}, {30, 512}, {15, 1024}, {10, 2048}
        };
        PoolManager* tcStore =new PoolManager(objects::TC_STORE, poolConfig);
        size_t additionalSize = 0;
        size_t storeSize = tcStore->getTotalSize(&additionalSize);
        sif::info << "Allocating " << storeSize + additionalSize << " bytes "
                <<"for the TC Store.." << std::endl;
	}


	/* Utility */
	new TimeStamper(objects::TIME_STAMPER);

	/* Distributor Tasks */
	new CCSDSDistributor(apid::SOURCE_OBSW,objects::CCSDS_PACKET_DISTRIBUTOR);
	new PUSDistributor(apid::SOURCE_OBSW,objects::PUS_PACKET_DISTRIBUTOR,
			objects::CCSDS_PACKET_DISTRIBUTOR);

	/* UDP Server */
#if defined(ETHERNET)
	new TmTcUdpBridge(objects::UDP_TMTC_BRIDGE, objects::CCSDS_PACKET_DISTRIBUTOR);
	new EmacPollingTask(objects::EMAC_POLLING_TASK); // used on both boards
#endif

	/* Serial TmTc Bridge */
	// 6 kB for shared ring buffer now.
	new SharedRingBuffer(objects::SERIAL_RING_BUFFER, 6144, true, 30);
	new TmTcSerialBridge(objects::SERIAL_TMTC_BRIDGE,
			objects::CCSDS_PACKET_DISTRIBUTOR, objects::TM_STORE,
			objects::TC_STORE, objects::SERIAL_RING_BUFFER);
	new RS232PollingTask(objects::SERIAL_POLLING_TASK,
	        objects::SERIAL_RING_BUFFER);

	/* TM Destination */
	new TmFunnel(objects::TM_FUNNEL);

	/* PUS Standalone Services using PusServiceBase */
	new Service1TelecommandVerification(objects::PUS_SERVICE_1_VERIFICATION,
	        apid::SOURCE_OBSW, pus::PUS_SERVICE_1, objects::TM_FUNNEL,
			config::OBSW_SERVICE_1_MQ_DEPTH);
	new Service3Housekeeping(objects::PUS_SERVICE_3_HOUSEKEEPING,
			apid::SOURCE_OBSW, pus::PUS_SERVICE_3);
	new Service5EventReporting(objects::PUS_SERVICE_5_EVENT_REPORTING,
			apid::SOURCE_OBSW, pus::PUS_SERVICE_5);
	new Service9CustomTimeManagement(objects::PUS_SERVICE_9_TIME_MGMT,
			apid::SOURCE_OBSW, pus::PUS_SERVICE_9);
	new Service17CustomTest(objects::PUS_SERVICE_17_TEST, apid::SOURCE_OBSW,
	        pus::PUS_SERVICE_17);


	/* PUS Gateway Services using CommandingServiceBase */
	new Service2DeviceAccess(objects::PUS_SERVICE_2_DEVICE_ACCESS,
			apid::SOURCE_OBSW, pus::PUS_SERVICE_2);
	new Service6MemoryManagement(objects::PUS_SERVICE_6_MEM_MGMT,
			apid::SOURCE_OBSW, pus::PUS_SERVICE_6);
    new Service8FunctionManagement(objects::PUS_SERVICE_8_FUNCTION_MGMT,
    		apid::SOURCE_OBSW, pus::PUS_SERVICE_8);
	new Service20ParameterManagement(objects::PUS_SERVICE_20_PARAM_MGMT);
    new Service23FileManagement(objects::PUS_SERVICE_23_FILE_MGMT, apid::SOURCE_OBSW,
            pus::PUS_SERVICE_23);
	new CService200ModeCommanding(objects::PUS_SERVICE_200_MODE_MGMT,
			apid::SOURCE_OBSW, pus::PUS_SERVICE_200);
	new CService201HealthCommanding(objects::PUS_SERVICE_201_HEALTH, apid::SOURCE_OBSW,
	        pus::PUS_SERVICE_201);

	/*
	 * Device Handlers using DeviceHandlerBase.
	 * These includes work without connected hardware via virtualized
	 * devices and interfaces
	 */
	CookieIF * dummyCookie0 = new TestCookie(addresses::PCDU);
	new PCDUHandler(objects::PCDU_HANDLER,objects::DUMMY_ECHO_COM_IF,
			dummyCookie0);
	CookieIF * dummyCookie1 = new TestCookie(addresses::DUMMY_ECHO);
	new TestDevice(objects::DUMMY_HANDLER, objects::DUMMY_ECHO_COM_IF,
			dummyCookie1, true);

	new CoreController(objects::CORE_CONTROLLER, objects::SYSTEM_STATE_TASK);
	new SystemStateTask(objects::SYSTEM_STATE_TASK, objects::CORE_CONTROLLER);
	new ThermalController(objects::THERMAL_CONTROLLER);

	TestCookie * dummyCookie2 = new TestCookie(addresses::DUMMY_GPS0);
#if defined(VIRTUAL_BUILD)
	new GPSHandler(objects::GPS0_HANDLER, objects::DUMMY_GPS_COM_IF,
	        dummyCookie2, switches::GPS0);
	TestCookie * dummyCookie3 = new TestCookie(addresses::DUMMY_GPS1);
	new GPSHandler(objects::GPS1_HANDLER, objects::DUMMY_GPS_COM_IF,
	        dummyCookie3, switches::GPS1);
#else
	new GPSHandler(objects::GPS0_HANDLER,objects::DUMMY_GPS_COM_IF,
			dummyCookie2, switches::GPS0);
	new GPSHandler(objects::GPS1_HANDLER,objects::DUMMY_GPS_COM_IF,
			dummyCookie2, switches::GPS1);
#endif

	/* Dummy Communication Interfaces */
	new TestEchoComIF(objects::DUMMY_ECHO_COM_IF);
	new DummyGPSComIF(objects::DUMMY_GPS_COM_IF);

	/* Test Tasks */
	new AtmelTestTask(objects::TEST_TASK);

	/* Board dependant files */

	new SoftwareImageHandler(objects::SOFTWARE_IMAGE_HANDLER);
	new SDCardHandler(objects::SD_CARD_HANDLER);
#ifdef ISIS_OBC_G20
	new FRAMHandler(objects::FRAM_HANDLER);
#endif

	/* Communication Interfaces */
//	new RS232DeviceComIF(objects::RS232_DEVICE_COM_IF);
	new I2cDeviceComIF(objects::I2C_DEVICE_COM_IF);
	new GpioDeviceComIF(objects::GPIO_DEVICE_COM_IF);
	new SpiDeviceComIF(objects::SPI_DEVICE_COM_IF);


	/* Test Tasks AT91 */
	//size_t I2C_MAX_REPLY_LEN = 256;
	size_t SPI_MAX_REPLY_LEN = 128;
	uint8_t delayBetweenChars = 5;
	uint8_t delayBeforeSpck = 2;

    CookieIF* spiCookie = nullptr;
    spiCookie = new SpiCookie(addresses::SPI_Test_PT1000 ,
            5, SlaveType::DEMULTIPLEXER_1, DemultiplexerOutput::OUTPUT_3,
            SPImode::mode1_spi, 5 , 3'900'000, 1);
    ThermalSensorHandler* thermalSensorHandler =  new ThermalSensorHandler(
    		objects::SPI_Test_PT1000, objects::SPI_DEVICE_COM_IF, spiCookie,
			switches::PT1000);
    thermalSensorHandler->setStartUpImmediately();

    spiCookie = new SpiCookie(addresses::SPI_Test_Gyro, 10, SlaveType::DEMULTIPLEXER_1,
    		DemultiplexerOutput::OUTPUT_4, SPImode::mode0_spi, 2, 3'900'000, 1);
    GyroHandler* gyroHandler = new GyroHandler(objects::SPI_Test_Gyro,
            objects::SPI_DEVICE_COM_IF, spiCookie, switches::GYRO1);
    gyroHandler->setStartUpImmediately();

#if OBSW_ADD_TEST_CODE == 1
	//CookieIF * i2cCookie_0 = new I2cCookie(addresses::I2C_ARDUINO_0,
	//        I2C_MAX_REPLY_LEN);
	spiCookie = new SpiCookie(addresses::SPI_ARDUINO_0,
	        SPI_MAX_REPLY_LEN, SlaveType::SLAVE_SELECT_0,
	        DemultiplexerOutput::OWN_SLAVE_SELECT, SPImode::mode0_spi,
	        delayBetweenChars, SPI_MIN_BUS_SPEED, delayBeforeSpck);
	new AtmelArduinoHandler(objects::ARDUINO_0, objects::SPI_DEVICE_COM_IF,
	        spiCookie, switches::DUMMY, std::string("Arduino 0"));

#ifdef ISIS_OBC_G20
	new LedTask(objects::LED_TASK, "IOBC_LED_TASK", LedTask::LedModes::WAVE_UP);
#else
	new LedTask(objects::LED_TASK, "AT91_LED_TASK", LedTask::LedModes::WAVE_UP);
#endif
	// Don't use UART0 test together with Serial Polling Task!!
	//new UART0TestTask("UART0 Test Task", objects::AT91_UART0_TEST_TASK);
	//new UART2TestTask("UART2 Test Task", objects::AT91_UART2_TEST_TASK);
	//new TwiTestTask(objects::AT91_I2C_TEST_TASK, 16);
    //new SpiTestTask(objects::AT91_SPI_TEST_TASK, SpiTestTask::SpiTestMode::GYRO);
#endif
}

void Factory::setStaticFrameworkObjectIds() {
	PusServiceBase::packetSource = objects::PUS_PACKET_DISTRIBUTOR;
	PusServiceBase::packetDestination = objects::TM_FUNNEL;

	CommandingServiceBase::defaultPacketSource = objects::PUS_PACKET_DISTRIBUTOR;
	CommandingServiceBase::defaultPacketDestination = objects::TM_FUNNEL;

	VerificationReporter::messageReceiver = objects::PUS_SERVICE_1_VERIFICATION;

	DeviceHandlerBase::powerSwitcherId = objects::PCDU_HANDLER;
	DeviceHandlerBase::rawDataReceiverId = objects::PUS_SERVICE_2_DEVICE_ACCESS;

	DeviceHandlerFailureIsolation::powerConfirmationId = objects::PCDU_HANDLER;

	TmPacketStored::timeStamperId = objects::TIME_STAMPER;
#if defined(ETHERNET)
	TmFunnel::downlinkDestination = objects::UDP_TMTC_BRIDGE;
#else
	TmFunnel::downlinkDestination = objects::SERIAL_TMTC_BRIDGE;
#endif
}

size_t Factory:: calculateStorage(uint8_t numberOfPools,
        uint16_t* numberOfElements,
        uint16_t* sizeOfElements) {
    size_t size = 0;
    for(uint8_t idx = 0; idx < numberOfPools; idx ++) {
        size += numberOfElements[idx] * sizeOfElements[idx];
    }
    return size;
}

