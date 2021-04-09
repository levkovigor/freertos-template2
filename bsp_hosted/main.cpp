#include <fsfw/tasks/TaskFactory.h>

#if defined(GCOV)
#include <boardconfig/gcov.h>
#endif

#include <iostream>
#include <OBSWVersion.h>

// This will be the entry to the mission specific code
void initMission();

#ifdef _WIN32
static const char* COMPILE_PRINTOUT = "Windows";
#elif defined(__unix__)
static const char* COMPILE_PRINTOUT = "Linux";
#else
static const char* COMPILE_PRINTOUT = "unknown OS";
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
    std::cout << "-- Software version v" << SW_VERSION << "." << SW_SUBVERSION << "." <<
            SW_SUBSUBVERSION << " -- " << std::endl;
    std::cout << "-- " <<  __DATE__ << " " << __TIME__ << " --" << std::endl;
    initMission();
    for(;;) {
        // suspend main thread by sleeping it.
        TaskFactory::delayTask(5000);
    }
}


