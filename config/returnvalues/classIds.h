#ifndef CONFIG_RETURNVALUES_CLASSIDS_H_
#define CONFIG_RETURNVALUES_CLASSIDS_H_

/**
 * @brief   CLASS_ID defintions which are required for custom returnvalues.
 */
namespace CLASS_ID {
enum {
	MISSION_CLASS_ID_START = FW_CLASS_ID_COUNT,
	RS232_CHANNEL, //!< RS232
	I2C_CHANNEL, //!< I2C
	SPI_CHANNEL, //!< SPI
	GPS_HANDLER,  //!< GPS
	PUS_SERVICE_3, //!< HKS
	SD_CARD_ACCESS, //!< SDCA
	SD_CARD_HANDLER, //!< SDCH
};
}


#endif /* CONFIG_RETURNVALUES_CLASSIDS_H_ */
