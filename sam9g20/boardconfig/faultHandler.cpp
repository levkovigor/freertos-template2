#include "faultHandler.h"
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <at91/utility/exithandler.h>
#include <at91/utility/trace.h>
#include <sam9g20/common/FRAMApi.h>


/*----------------------------------------------------------------------------
 *        Constants
 *----------------------------------------------------------------------------*/

#ifdef DEBUG

/* IFSR status */
static const char* _prefetch_abort_status[32] = {
	NULL,
	NULL,
	"debug event",
	"access flag fault, section",
	NULL,
	"translation fault, section",
	"access flag fault, page",
	"translation fault, page",
	"synchronous external abort",
	"domain fault, section",
	NULL,
	"domain fault, page",
	"L1 translation, synchronous external abort",
	"permission fault, section",
	"L2 translation, synchronous external abort",
	"permission fault, page",
};

/* DFSR status */
static const char* _data_abort_status[32] = {
	NULL,
	"alignment fault",
	"debug event",
	"access flag fault, section",
	"instruction cache maintenance fault",
	"translation fault, section",
	"access flag fault, page",
	"translation fault, page",
	"synchronous external abort, nontranslation",
	"domain fault, section",
	NULL,
	"domain fault, page",
	"1st level translation, synchronous external abort",
	"permission fault, section",
	"2nd level translation, synchronous external abort",
	"permission fault, page",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"asynchronous external abort"
};

#endif /* DEBUG */

//------------------------------------------------------------------------------
/// Default spurious interrupt handler. Infinite loop.
//------------------------------------------------------------------------------
void defaultSpuriousHandler( void )
{
	TRACE_DEBUG("UNEXPECTED SPURIOUS INTERRUPT OCCURRED, HALTING! \n\r");
	while (1);
}

//------------------------------------------------------------------------------
/// Default handler for fast interrupt requests. Infinite loop.
//------------------------------------------------------------------------------
void defaultFiqHandler( void )
{
	TRACE_FATAL("UNEXPECTED FIQ INTERRUPT OCCURRED, HALTING! \n\r");
    while (1);
}

//------------------------------------------------------------------------------
/// Default handler for standard interrupt requests. Infinite loop.
//------------------------------------------------------------------------------
void defaultIrqHandler( void )
{
	TRACE_FATAL("UNEXPECTED INTERRUPT OCCURRED, HALTING! \n\r");
    while (1);
}

/*----------------------------------------------------------------------------
 *        Functions
 *----------------------------------------------------------------------------*/

/**
 * @brief Default handler for "Data Abort" exception
 */
extern "C" void data_abort_irq_handler(void)
{
#ifdef DEBUG
	uint32_t v1, v2, dfsr;

	asm("mrc p15, 0, %0, c5, c0, 0" : "=r"(v1));
	asm("mrc p15, 0, %0, c6, c0, 0" : "=r"(v2));
	printf("\r\n");
	TRACE_ERROR("####################\r\n");
	dfsr = ((v1 >> 4) & 0x0F);
	TRACE_ERROR("Data Fault occured in %x domain\r\n", (unsigned int)dfsr);
	dfsr = (((v1 & 0x400) >> 6) | (v1 & 0x0F));
	if (_data_abort_status[dfsr]) {
		TRACE_ERROR("Data Fault reason is: %s\r\n", _data_abort_status[dfsr]);
	}

	else {
		TRACE_ERROR("Data Fault reason is unknown\r\n");
	}
	TRACE_ERROR("Data Fault occured at address: 0x%08x\r\n", (unsigned int)v2);
	TRACE_ERROR("Data Fault status register value: 0x%x\r\n", (unsigned int)v1);

	// We could also write some error information into FRAM here.
	TRACE_ERROR("####################\n\r");
#else
	TRACE_ERROR("\r\nDATA ABORT EXCEPTION OCCURED! HALTING! \n\r");
#endif

    // Increment reboot counter in FRAM
#ifdef ISIS_OBC_G20
    increment_reboot_counter(false, false);
#endif

	// Call ISIS handler which also restarts the CPU
	restartDataAbort();

	while(1);
}

/**
 * @brief Default handler for "Prefetch Abort" exception
 */
extern "C" void prefetch_abort_irq_handler(void)
{
#ifdef DEBUG
	uint32_t v1, v2, ifsr;

	asm("mrc p15, 0, %0, c5, c0, 1" : "=r"(v1));
	asm("mrc p15, 0, %0, c6, c0, 2" : "=r"(v2));

	printf("\r\n");
	TRACE_ERROR("####################\r\n");
	ifsr = (((v1 & 0x400) >> 6) | (v1 & 0x0F));
	if (_prefetch_abort_status[ifsr]) {
		TRACE_ERROR("Prefetch Fault reason is: %s\r\n", _prefetch_abort_status[ifsr]);
	}
	else {
		TRACE_ERROR("Prefetch Fault reason is unknown\r\n");
	}
	TRACE_ERROR("prefetch Fault occured at address: 0x%08x\r\n", (unsigned int)v2);
	TRACE_ERROR("Prefetch Fault status register value: 0x%x\r\n", (unsigned int)v1);

	TRACE_ERROR("####################\n\r");
#else
	TRACE_ERROR("\r\nPREFETCH ABORT EXCEPTION OCCURED! HALTING! \n\r");
#endif

    // Increment reboot counter in FRAM
#ifdef ISIS_OBC_G20
	increment_reboot_counter(false, false);
#endif

	// Call ISIS handler which also restarts the CPU
	restartPrefetchAbort();

	while(1);
}
