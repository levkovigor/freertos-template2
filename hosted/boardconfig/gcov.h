#ifndef LINUX_GCOV_H_
#define LINUX_GCOV_H_
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>

#ifdef GCOV
extern "C" void __gcov_flush();
#else
void __gcov_flush() {
	sif::info << "GCC GCOV: Please supply GCOV=1 in Makefile if "
			"coverage information is desired.\n" << std::flush;
}
#endif

#endif /* LINUX_GCOV_H_ */
