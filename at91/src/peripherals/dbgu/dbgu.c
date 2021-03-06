/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include <AT91SAM9G20.h>
#include <board.h>
#include <at91/peripherals/dbgu/dbgu.h>
#include <stdarg.h>


//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/// Initializes the DBGU with the given parameters, and enables both the
/// transmitter and the receiver. The mode parameter contains the value of the
/// DBGU_MR register.
/// Value DBGU_STANDARD can be used for mode to get the most common configuration
/// (i.e. aysnchronous, 8bits, no parity, 1 stop bit, no flow control).
/// \param mode  Operating mode to configure.
/// \param baudrate  Desired baudrate (e.g. 115200).
/// \param mck  Frequency of the system master clock in Hz.
//------------------------------------------------------------------------------
void DBGU_Configure(
    unsigned int mode,
    unsigned int baudrate,
    unsigned int mck)
{
    // Reset & disable receiver and transmitter, disable interrupts
    AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RSTRX | AT91C_US_RSTTX;
    AT91C_BASE_DBGU->DBGU_IDR = 0xFFFFFFFF;

    // Configure baud rate
    AT91C_BASE_DBGU->DBGU_BRGR = mck / (baudrate * 16);

    // Configure mode register
    AT91C_BASE_DBGU->DBGU_MR = mode;

    // Disable DMA channel
    AT91C_BASE_DBGU->DBGU_PTCR = AT91C_PDC_RXTDIS | AT91C_PDC_TXTDIS;

    // Enable receiver and transmitter
    AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RXEN | AT91C_US_TXEN;
}

//------------------------------------------------------------------------------
/// Outputs a character on the DBGU line.
/// \note This function is synchronous (i.e. uses polling).
/// \param c  Character to send.
//------------------------------------------------------------------------------
void DBGU_PutChar(unsigned char c)
{
	/*
	 * US_TXEMPTY is not a blocking register. The UART shift register will shift
	 * the contents out regardless. No need to add watchdog kick. However, might
	 * be good idea to add a normal "for loop" timeout counter just to be safe.
	 *
	 * int timeout;
	 * for( timeout = 1000000; timeout != 0; timeout-- );
	 */

    // Wait for the transmitter to be ready
    while ((AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXEMPTY) == 0) {
//    	WDT_kickEveryNcalls(40000);
    }

    // Send character
    AT91C_BASE_DBGU->DBGU_THR = c;

    // Wait for the transfer to complete
    while ((AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXEMPTY) == 0) {
//    	WDT_kickEveryNcalls(40000);
    }
}

//------------------------------------------------------------------------------
/// Return 1 if a character can be read in DBGU
//------------------------------------------------------------------------------
unsigned int DBGU_IsRxReady()
{
    return (AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_RXRDY);
}

//------------------------------------------------------------------------------
/// Reads and returns a character from the DBGU.
/// \note This function is synchronous (i.e. uses polling).
/// \return Character received.
//------------------------------------------------------------------------------
unsigned char DBGU_GetChar(void)
{
    while ((AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_RXRDY) == 0)
    {
//#ifdef norflash
//    	WDT_kickEveryNcalls(8000);
//#else
//    	WDT_kickEveryNcalls(40000);
//#endif

    	/*
    	 * If the user properly checked DBGU_IsRxRead() before calling this
    	 * function, there is no need to add timeout here. You could even argue
    	 * that you would want the watchdog to trigger, alerting the user to
    	 * this fact.
    	 */
    }

    return AT91C_BASE_DBGU->DBGU_RHR;
}

/*
 * DBGU_GetCharTimeout(...) adds a lot of dependencies (HAL & FreeRTOS). By
 * rather using DBG_IsRxReady() together with DBGU_GetChar(), the user can
 * recreate this functionality in an upper layer.
 */

#ifndef NOFPUT
#include <stdio.h>
#include <unistd.h>

#ifndef USE_AT91LIB_STDIO_AND_STRING
/**
 * Reimplemented the lowlevel write() call so newlib printf prints to DBGU.
 * See: https://www.gnu.org/software/libc/manual/html_node/I_002fO-Primitives.html#I_002fO-Primitives.
 * For some reason, _write() is called instead of POSIX write() in newlib.
 */
int write(int __fd, const void *__buf, size_t __nbyte)
{
	return _write(__fd, __buf, __nbyte);
}

#endif
//------------------------------------------------------------------------------
/// \exclude
/// Implementation of fputc using the DBGU as the standard output. Required
/// for printf().
/// \param c  Character to write.
/// \param pStream  Output stream.
/// \param The character written if successful, or -1 if the output stream is
/// not stdout or stderr.
//------------------------------------------------------------------------------
signed int fputc(signed int c, FILE *pStream)
{
    if ((pStream == stdout) || (pStream == stderr)) {

        DBGU_PutChar(c);
        return c;
    }
    else {

        return EOF;
    }
}

//------------------------------------------------------------------------------
/// \exclude
/// Implementation of fputs using the DBGU as the standard output. Required
/// for printf(). Does NOT currently use the PDC.
/// \param pStr  String to write.
/// \param pStream  Output stream.
/// \return Number of characters written if successful, or -1 if the output
/// stream is not stdout or stderr.
//------------------------------------------------------------------------------
signed int fputs(const char *pStr, FILE *pStream)
{
    signed int num = 0;

    while (*pStr != 0) {

        if (fputc(*pStr, pStream) == -1) {

            return -1;
        }
        num++;
        pStr++;
    }

    return num;
}

#undef putchar

//------------------------------------------------------------------------------
/// \exclude
/// Outputs a character on the DBGU.
/// \param c  Character to output.
/// \return The character that was output.
//------------------------------------------------------------------------------
signed int putchar(signed int c)
{
    return fputc(c, stdout);
}

#endif //#ifndef NOFPUT

