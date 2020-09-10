/*
 * ethernetif.c
 *
 *  Created on: 02.08.2019
 *      Author: Jakob Meier
 */

#include <lwip/mem.h>
#include <lwip/memp.h>
#include <lwip/netif.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <lwip/ip_addr.h>
#include <lwip/etharp.h>
#include <lwip/init.h>
#include <sam9g20/at91/include/at91/boards/at91sam9g20-ek/drivers/emac/emac.h>
#include <sam9g20/at91/include/at91/boards/at91sam9g20-ek/drivers/macb/macb.h>
#include <sam9g20/at91/include/at91/boards/at91sam9g20-ek/drivers/macb/mii.h>
#include <sam9g20/at91/include/at91/boards/at91sam9g20-ek/ethernet/emacif.h>
#include <sam9g20/at91/include/at91/boards/at91sam9g20-ek/ethernet/lwip_init.h>
#include <AT91SAM9G20.h>
#include <at91/peripherals/aic/aic.h>
#include <at91/peripherals/pio/pio.h>
#include <at91/utility/trace.h>


ip_addr_t ipaddr, netmask, gw;
struct netif NetIf, *netif;

Macb mac;
bool ethernetCableConnected;
 /* The PINs' on PHY reset */
 static const Pin emacRstPins[] = {BOARD_EMAC_RST_PINS}; // @suppress("Symbol is not resolved")

 /* The PINs for EMAC */
 static const Pin emacPins[] = {BOARD_EMAC_RUN_PINS}; // @suppress("Symbol is not resolved")

 void emac_lwip_init(){

	 Macb* pMac = &mac;

	 /* Display MAC & IP settings */
	 printf("-I- MAC %x:%x:%x:%x:%x:%x\n\r",
			MacAddress[0], MacAddress[1], MacAddress[2],
			MacAddress[3], MacAddress[4], MacAddress[5]);
	 printf("-I- Host IP  %d.%d.%d.%d\n\r",
			IpAddress[0], IpAddress[1],
			IpAddress[2], IpAddress[3]);
	 printf("-I- Net Mask  %d.%d.%d.%d\n\r",
			NetMask[0], NetMask[1], NetMask[2], NetMask[3]);

	 /* peripheral id of emac is 21
	  * copy all frames bit, set to 1
	  * NBC set to 1 (When set to 1, frames addressed to the broadcast address of all ones are not received)
	  */
	 EMAC_Init(21, MacAddress, EMAC_CAF_ENABLE, EMAC_NBC_ENABLE);
	 MACB_Init(pMac, BOARD_EMAC_PHY_ADDR); // @suppress("Symbol is not resolved")

	 /* PHY initialization */
	 if (!MACB_InitPhy(pMac, BOARD_MCK, emacRstPins, PIO_LISTSIZE(emacRstPins), emacPins, PIO_LISTSIZE(emacPins))) {
		 TRACE_ERROR("PHY Initialization ERROR!\n\r");
	 }

	 /* Auto Negotiate */
	if (!MACB_AutoNegotiate(pMac)) {
		TRACE_INFO("Auto Negotiate unsuccessfull!\n\r");
		ethernetCableConnected = false;
	} else {
		ethernetCableConnected = true;
	}

	/* Setup EMAC buffers and interrupts */
	AIC_ConfigureIT(21, AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, ISR_Emac);
	AIC_EnableIT(21);

	/* Initialize lwIP modules */
	lwip_init();

	/* Initialize net interface for lwIP */
	emacif_setmac((uint8_t*) MacAddress);

	IP4_ADDR(&gw, GateWay[0], GateWay[1], GateWay[2], GateWay[3]);
	IP4_ADDR(&ipaddr, IpAddress[0], IpAddress[1], IpAddress[2], IpAddress[3]);
	IP4_ADDR(&netmask, NetMask[0], NetMask[1], NetMask[2], NetMask[3]);

	netif = netif_add(&NetIf, &ipaddr, &netmask, &gw, NULL, emacif_init,
			ip_input);
	netif_set_default(&NetIf);
	netif_set_up(&NetIf);
	netif_set_link_up(&NetIf);
 }

/* emac interrupt handler */
 void ISR_Emac(){
     EMAC_Handler();
 }
