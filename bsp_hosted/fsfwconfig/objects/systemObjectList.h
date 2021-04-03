#ifndef HOSTED_CONFIG_OBJECTS_SYSTEMOBJECTLIST_H_
#define HOSTED_CONFIG_OBJECTS_SYSTEMOBJECTLIST_H_

#include <cstdint>

// The objects will be instantiated in the ID order
namespace objects {
	enum sourceObjects: uint32_t {
		/* First Byte 0x50-0x52 reserved for PUS Services **/
		CCSDS_PACKET_DISTRIBUTOR = 0x50000100,
		PUS_PACKET_DISTRIBUTOR = 0x50000200,
		UDP_BRIDGE = 0x50000300,
		UDP_POLLING_TASK = 0x50000400,

		ATTITUDE_CONTROLLER = 0x40003000,

		PUS_SERVICE_3 = 0x51000300,
		PUS_SERVICE_6_MEM_MGMT = 0x51000500,
		PUS_SERVICE_11_TC_SCHED = 0x51001100,
		PUS_SERVICE_23_FILE_MGMT = 0x51002300,
		PUS_SERVICE_201_HEALTH = 0x51020100,

		PUS_TIME = 0x52000001,
		TM_FUNNEL = 0x52000002,

		// Devices connected to decoder 1
		SPI_Test_PT1000 = 0x44115400,
		SPI_Test_Gyro = 0x44115500,
		PT1000_Syrlinks_DEC1_O1 = 0x44115401,
		PT1000_Camera_DEC1_O2 = 0x44115402,
		PT1000_SuS1_DEC1_O3 = 0x44115404,
		PT1000_SuS2_DEC1_O4 = 0x44115405,
		PT1000_SuS3_DEC1_O5 = 0x44115406,
		PT1000_PVHC_DEC1_O6 = 0x44115407,

		// Devices connected to decoder 2
		PT1000_CCSDS1_DEC2 = 0x44125401,
		PT1000_MGT1_DEC2 = 0x44125403,
		PT1000_SuS4_DEC2 = 0x44125404,
		PT1000_SuS5_DEC2 = 0x44125405,
		PT1000_SuS6_DEC2 = 0x44125406,
		PT1000_PVCH_DEC2 = 0x44125407,
		SuS_ADC1_DEC2 = 0x44020108,

		// Devices connected to decoder 3
		PT1000_Iridium_DEC3 = 0x44130301,
		PT1000_CCSDS2_DEC3 = 0x44130302,
		PT1000_SuS7_DEC3 = 0x44130305,
		PT1000_SuS8_DEC3 = 0x44130306,
		PT1000_PVCH_DEC3 = 0x44130307,
		GYRO2 = 0x44130308,

		// Devices connected to decoder 4
		PT1000_PLOC_DEC4 = 0x44145401,
		PT1000_SuS9_DEC4 = 0x44145404,
		PT1000_SuS10_DEC4 = 0x44145405,
		PT1000_PVHC_DEC4 = 0x44145406,
		SuS_ADC_DEC4 = 0x44145407,

		/* 0x49 ('I') for Communication Interfaces **/
		DUMMY_ECHO_COM_IF = 0x4900AFFE,
		DUMMY_GPS_COM_IF = 0x49001F00,
		RS232_DEVICE_COM_IF = 0x49005200,
		I2C_DEVICE_COM_IF = 0x49005300,
		GPIO_DEVICE_COM_IF = 0x49005400,
		SPI_DEVICE_COM_IF = 0x49005600,
		//SPI_POLLING_TASK = 0x49005410,

		/* Test Task */
		TEST_TASK = 0x42694269,
		DUMMY_INTERFACE = 0xCAFECAFE,
		DUMMY_HANDLER = 0x4400AFFE,
		TC_INJECTOR = 0x99000001
	};
}

#endif /* BSP_CONFIG_OBJECTS_SYSTEMOBJECTLIST_H_ */

/**
                   ▄              ▄
                  ▌▒█           ▄▀▒▌
                  ▌▒▒█        ▄▀▒▒▒▐
                 ▐▄▀▒▒▀▀▀▀▄▄▄▀▒▒▒▒▒▐
               ▄▄▀▒░▒▒▒▒▒▒▒▒▒█▒▒▄█▒▐
             ▄▀▒▒▒░░░▒▒▒░░░▒▒▒▀██▀▒▌
            ▐▒▒▒▄▄▒▒▒▒░░░▒▒▒▒▒▒▒▀▄▒▒▌
            ▌░░▌█▀▒▒▒▒▒▄▀█▄▒▒▒▒▒▒▒█▒▐
           ▐░░░▒▒▒▒▒▒▒▒▌██▀▒▒░░░▒▒▒▀▄▌
           ▌░▒▄██▄▒▒▒▒▒▒▒▒▒░░░░░░▒▒▒▒▌
          ▌▒▀▐▄█▄█▌▄░▀▒▒░░░░░░░░░░▒▒▒▐
          ▐▒▒▐▀▐▀▒░▄▄▒▄▒▒▒▒▒▒░▒░▒░▒▒▒▒▌
          ▐▒▒▒▀▀▄▄▒▒▒▄▒▒▒▒▒▒▒▒░▒░▒░▒▒▐
           ▌▒▒▒▒▒▒▀▀▀▒▒▒▒▒▒░▒░▒░▒░▒▒▒▌
           ▐▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒░▒░▒▒▄▒▒▐
            ▀▄▒▒▒▒▒▒▒▒▒▒▒░▒░▒░▒▄▒▒▒▒▌
              ▀▄▒▒▒▒▒▒▒▒▒▒▄▄▄▀▒▒▒▒▄▀
                ▀▄▄▄▄▄▄▀▀▀▒▒▒▒▒▄▄▀
                   ▒▒▒▒▒▒▒▒▒▒▀▀
**/
