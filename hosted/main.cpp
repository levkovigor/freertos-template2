#if defined(GCOV)
#include <gcov.h>
#endif

#include <iostream>

// This will be the entry to the mission specific code
void initMission();

#ifdef WIN32
static const char* COMPILE_PRINTOUT = "Windows";
#elif LINUX
static const char* COMPILE_PRINTOUT = "Linux";
#else
static const char* COMPILE_PRINTOUT = "unknown OS";
#endif

#ifndef SW_VERSION
#define SW_VERSION 0
#endif

#ifndef SW_SUBVERSION
#define SW_SUBVERSION 0
#endif

/**
 * @brief 	This is the main program for the hosted build. It can be run for
 * 			Linux and Windows.
 * @return
 */
int main(void)
{
    std::cout << "-- SOURCE Hosted OBSW --" << std::endl;
    std::cout << "-- Compiled for " << COMPILE_PRINTOUT << " --" << std::endl;
    std::cout << "-- Software version v" << SW_VERSION << "." << SW_SUBVERSION
            << " -- " << std::endl;
    std::cout << "-- " <<  __DATE__ << " " << __TIME__ << " --" << std::endl;
	initMission();
	for(;;) {}
}


