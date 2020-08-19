#if defined(GCOV)
#include <gcov.h>
#endif


// This will be the entry to the mission specific code
void initMission();


/**
 * @brief 	This is the main program for the hosted build. It can be run for
 * 			Linux and Windows.
 * @return
 */
int main(void)
{
	initMission();
	for(;;) {}
}


