#include "bootloaderConfig.h"

#ifdef AT91SAM9G20_EK
void at91_main();
#elif defined(ISIS_OBC_G20)
void iobc_main();
#endif

/**
 * @brief	Bootloader which will copy the primary software to SDRAM and
 * 			execute it
 * @author 	R. Mueller
 */
int main()
{
#ifdef AT91SAM9G20_EK
    at91_main();
#elif defined(ISIS_OBC_G20)
    iobc_main();
#else
    // Debug output would be good.
    return 0;
#endif
}



