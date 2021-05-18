#ifndef FSFWCONFIG_OBJECTS_SYSTEMOBJECTLIST_H_
#define FSFWCONFIG_OBJECTS_SYSTEMOBJECTLIST_H_

#include "common/objects/commonObjectsList.h"
#include "fsfw/objectmanager/frameworkObjects.h"

#include <cstdint>

namespace objects {
enum sourceObjects: object_id_t {
    /* UDP ID must come after CCSDS Distributor  **/
    UDP_TMTC_BRIDGE = 0x50000300,
    EMAC_POLLING_TASK = 0x50000400,
    SERIAL_TMTC_BRIDGE = 0x50000500,
    SERIAL_RING_BUFFER = 0x50000550,
    SERIAL_POLLING_TASK = 0x50000600,

    /* 0x43 ('C') for Controllers */
    CORE_CONTROLLER = 0x43001000,
    SYSTEM_STATE_TASK = 0x43001005,
    RS485_CONTROLLER = 0x43005000,

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
