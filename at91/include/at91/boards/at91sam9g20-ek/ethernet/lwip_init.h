/*
 * ethernetif.h
 *
 *  Created on: 02.08.2019
 *      Author: Jakob Meier
 */

#ifndef AT91_INCLUDE_AT91_BOARDS_AT91SAM9G20_EK_ETHERNET_ETHERNETIF_H_
#define AT91_INCLUDE_AT91_BOARDS_AT91SAM9G20_EK_ETHERNET_ETHERNETIF_H_
#include <stdbool.h>

extern bool ethernetCableConnected;

// mac address
static const unsigned char MacAddress[6] = {0x20, 0x00, 0x00, 0x00, 0x00, 0x00};

// ipaddress
static const unsigned char IpAddress[4] = {169, 254, 1, 38};

// Set the default router's IP address.
static const unsigned char GateWay[4] = {192, 168, 178, 1};

// netmask
static const unsigned char NetMask[4] = {255, 255, 0, 0};

void emac_lwip_init();
void ISR_Emac();



#endif /* AT91_INCLUDE_AT91_BOARDS_AT91SAM9G20_EK_ETHERNET_ETHERNETIF_H_ */
