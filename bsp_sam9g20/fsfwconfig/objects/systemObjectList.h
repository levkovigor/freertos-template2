#ifndef FSFWCONFIG_OBJECTS_SYSTEMOBJECTLIST_H_
#define FSFWCONFIG_OBJECTS_SYSTEMOBJECTLIST_H_

#include <cstdint>

// The objects will be instantiated in the ID order
namespace objects {
	enum sourceObjects: uint32_t {
		/* First Byte 0x50-0x52 reserved for PUS Services **/
		CCSDS_PACKET_DISTRIBUTOR = 0x50000100,
		PUS_PACKET_DISTRIBUTOR = 0x50000200,
		/* UDP ID must come after CCSDS Distributor  **/
		UDP_TMTC_BRIDGE = 0x50000300,
		EMAC_POLLING_TASK = 0x50000400,
		SERIAL_TMTC_BRIDGE = 0x50000500,
		SERIAL_RING_BUFFER = 0x50000550,
		SERIAL_POLLING_TASK = 0x50000600,

		PUS_SERVICE_6_MEM_MGMT = 0x51000500,
		PUS_SERVICE_20_PARAM_MGMT = 0x51002000,
		PUS_SERVICE_23_FILE_MGMT = 0x51002300,
		PUS_SERVICE_201_HEALTH = 0x51020100,

		TM_FUNNEL = 0x52000002,
		FREERTOS_TASK_MONITOR = 0x53000003,

		CORE_CONTROLLER = 0x40001000,
        SYSTEM_STATE_TASK = 0x40001005,
        THERMAL_CONTROLLER = 0x40002000,
        ATTITUDE_CONTROLLER = 0x40003000,
		RS485_CONTROLLER = 0x40005000,

        /* 0x44 ('D') for Device Handlers **/
        /* Second Byte: ComIF -> 0x00: UART,0x10 SPI,0x20: I2C,30: GPIO,40: PWM */
        /* Third Byte: Device ID */
		PCDU_HANDLER = 0x44003200,
		GPS0_HANDLER = 0x44101F00,
		GPS1_HANDLER = 0x44202000,

		DLR_PVCH = 0x44104000,
		GYRO1 = 0x44105000,
		DLR_IRAS = 0x44106000,

		/* fourth byte decoder output ID */

		// Devices connected to decoder 1
		SPI_Test_PT1000 = 0x44115000,
		SPI_Test_Gyro = 0x44115001,
		SPI_Test_MGM = 0x44115002,

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

		/* 0x4d ('M') for Memory Handlers **/
		SD_CARD_HANDLER = 0x4D0073AD,
		FRAM_HANDLER = 0x4D008000,
		SOFTWARE_IMAGE_HANDLER = 0x4D009000,

		/* 0x62 ('b') for board specific objects */
		LED_TASK = 0x62000001,
		AT91_I2C_TEST_TASK = 0x62007401,
		AT91_UART0_TEST_TASK =  0x62007402,
		AT91_UART2_TEST_TASK = 0x62007403,
		AT91_SPI_TEST_TASK = 0x62007404,


		/* 0x74 ('t') for Test objects  */
		ARDUINO_0 = 0x01010100,
		ARDUINO_1 = 0x01010101,
		ARDUINO_2 = 0x01010102,
		ARDUINO_3 = 0x01010103,
		ARDUINO_4 = 0x01010104,
		TEST_TASK = 0x42694269,
		DUMMY_HANDLER_0 = 0x4400AFFE,
        DUMMY_HANDLER_1 = 0x4401AFFE,
		TC_INJECTOR	= 0x99000001
	};
}

#endif /* BSP_FSFWCONFIG_OBJECTS_SYSTEMOBJECTLIST_H_ */

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
