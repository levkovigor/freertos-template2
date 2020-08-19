#include "main.h"
#include <core/bootNorFlash.h>

#include <board.h>
#include <AT91SAM9G20.h>
#include <cp15/cp15.h>
#include <utility/trace.h>
#include <peripherals/dbgu/dbgu.h>
#include <peripherals/pio/pio.h>
#include <peripherals/pit/pit.h>
#include <peripherals/aic/aic.h>
#include <peripherals/pio/pio.h>

#include <stdbool.h>
#include <string.h>

void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType);


/**
 * @brief	Bootloader which will copy the primary software to SDRAM and
 * 			execute it
 * @author 	R. Mueller
 */
int main()
{
    //-------------------------------------------------------------------------
    // Configure traces
    //-------------------------------------------------------------------------
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);

    TRACE_INFO_WP("\n\r");
    TRACE_INFO_WP("-- SOURCE Bootloader --\n\r");
    TRACE_INFO_WP("-- %s\n\r", BOARD_NAME);
    TRACE_INFO_WP("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

    //-------------------------------------------------------------------------
    // Enable I-Cache
    //-------------------------------------------------------------------------
    CP15_Enable_I_Cache();

    // verify hamming code of image in sdram. code size is either written in
    // memory or extracted from FRAM.

    // if successfull, copy norflash to sdram
//    int result = copy_norflash_binary_to_sdram("Test", 256);
//    if(result != 0) {
//        // error
//    }

    // otherwise, try to copy SDCard binary to SDRAM

    for(;;);
}

void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType)
{
    typedef void(*fctType)(volatile unsigned int, volatile unsigned int);
    void(*pFct)(volatile unsigned int r0_val, volatile unsigned int r1_val);

    pFct = (fctType)jumpAddr;
    pFct(0/*dummy value in r0*/, matchType/*matchType in r1*/);

    while(1);//never reach
}



