/**
 * \file logicalAddresses.cpp
 *
 * \date 07.11.2019
 */

#ifndef CONFIG_DEVICES_LOGICALADDRESSES_H_
#define CONFIG_DEVICES_LOGICALADDRESSES_H_
#include <stdint.h>
#include <fsfw/devicehandlers/CookieIF.h>
#include <stm32/fsfwconfig/objects/systemObjectList.h>

namespace addresses {
	/* Logical addresses have uint32_t datatype */
	enum logicalAddresses: address_t {
		PCDU,
		/* I2C Addresses go from 0 to 128 ! */
		I2C_MGM = 0x1E,

		/* SPI Addresses (= Object Address) */
		SPI_DLR_PVCH = objects::DLR_PVCH,
		SPI_GYRO1 = objects::GYRO1,
		SPI_DLR_IRAS = objects::DLR_IRAS,

		/* fourth byte decoder output ID */

		// Devices connected to decoder 1
		SPI_PT1000_Syrlinks_DEC1_O1 = objects::PT1000_Syrlinks_DEC1_O1,
		SPI_PT1000_Camera_DEC1_O2 = objects::PT1000_Camera_DEC1_O2,
		SPI_PT1000_SuS1_DEC1_O3 = objects::PT1000_SuS1_DEC1_O3,
		SPI_PT1000_SuS2_DEC1_O4 = objects::PT1000_SuS2_DEC1_O4 ,
		SPI_PT1000_SuS3_DEC1_O5 = objects::PT1000_SuS3_DEC1_O5,
		SPI_PT1000_PVHC_DEC1_O6 = objects::PT1000_PVHC_DEC1_O6,

		// Devices connected to decoder 2
		SPI_PT1000_CCSDS1_DEC2 = objects::PT1000_CCSDS1_DEC2,
		SPI_PT1000_MGT1_DEC2 = objects::PT1000_MGT1_DEC2,
		SPI_PT1000_SuS4_DEC2 = objects::PT1000_SuS4_DEC2,
		SPI_PT1000_SuS5_DEC2 = objects::PT1000_SuS5_DEC2,
		SPI_PT1000_SuS6_DEC2 = objects::PT1000_SuS6_DEC2,
		SPI_PT1000_PVCH_DEC2 = objects::PT1000_PVCH_DEC2,
		SPI_SuS_ADC1_DEC2 = objects::SuS_ADC1_DEC2,

		// Devices connected to decoder 3
		SPI_PT1000_Iridium_DEC3 = objects::PT1000_Iridium_DEC3,
		SPI_PT1000_CCSDS2_DEC3 = objects::PT1000_CCSDS2_DEC3,
		SPI_PT1000_SuS7_DEC3 = objects::PT1000_SuS7_DEC3,
		SPI_PT1000_SuS8_DEC3 = objects::PT1000_SuS8_DEC3,
		SPI_PT1000_PVCH_DEC3 = objects::PT1000_PVCH_DEC3,
		SPI_GYRO2 = objects::GYRO2,

		// Devices connected to decoder 4
		SPI_PT1000_PLOC_DEC4 = objects::PT1000_PLOC_DEC4,
		SPI_PT1000_SuS9_DEC4 = objects::PT1000_SuS9_DEC4,
		SPI_PT1000_SuS10_DEC4 = objects::PT1000_SuS10_DEC4,
		SPI_PT1000_PVHC_DEC4 = objects::PT1000_PVHC_DEC4,
		SPI_SuS_ADC_DEC4 = objects::SuS_ADC_DEC4,
		SPI_PT1000_SuS1_DEC1 = objects::PT1000_SuS1_DEC1_O3,
		SPI_PT1000_SuS2_DEC1 = objects::PT1000_SuS2_DEC1_O4,

		/* GPIO Addresses (see board.h) */
		/* Rename according to what they are used for */
		GPIO05 = 0x0500210b, //!< EK: PIOB 21, iOBC: ?
		GPIO06 = 0x0600220b, //!< EK: PIOB 22, iOBC: ?
		GPIO07 = 0x0700230b,
		GPIO08 = 0x0800240b,
		GPIO09 = 0x0900250b,
		GPIO10 = 0x1000260b,
		GPIO11 = 0x1100270b,
		GPIO12 = 0x1200280b,
		GPIO13 = 0x1300290b,
		GPIO14 = 0x1400300b,
		GPIO15 = 0x1500310b,
		GPIO16 = 0x1600120a,
		GPIO17 = 0x1700130a,
		GPIO18 = 0x1800140a,
		GPIO19 = 0x1900150a,
		GPIO20 = 0x2000160a,
		GPIO21 = 0x2100170a,
		GPIO22 = 0x2200180a,
		GPIO23 = 0x2300190a,
		GPIO24 = 0x2400200a,
		GPIO25 = 0x2500210a,
		GPIO26 = 0x2600290a,

		/* Dummy and Test Addresses */
		DUMMY_ECHO = 129,
		DUMMY_GPS0  = 130,
		DUMMY_GPS1 = 131,
		I2C_ARDUINO_0 = 16, //!< I2C Address of Arduino
		I2C_ARDUINO_1 = 17,
		I2C_ARDUINO_2 = 18,
		I2C_ARDUINO_3 = 19,
		I2C_ARDUINO_4 = 20,
		SPI_ARDUINO_0 = objects::ARDUINO_0,
		SPI_ARDUINO_1 = objects::ARDUINO_1,
		SPI_ARDUINO_2 = objects::ARDUINO_2,
		SPI_ARDUINO_3 = objects::ARDUINO_3,
		SPI_ARDUINO_4 = objects::ARDUINO_4,
	};
}


#endif /* CONFIG_DEVICES_LOGICALADDRESSES_H_ */