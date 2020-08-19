/**
 * @file 	CatchSource.cpp
 * @brief 	Source file to compile catch framework.
 * @details	All tests should be written in other files.
 * For eclipse console output, install ANSI Escape in Console
 * from the eclipse market place to get colored characters.
 */

#ifndef NO_UNIT_TEST_FRAMEWORK

#define CATCH_CONFIG_RUNNER
#include <unittest/hosted/catch2/catch.hpp>
#include <unittest/hosted/CatchDefinitions.h>

#include <unittest/hosted/CatchMain.h>
#include <config/cdatapool/dataPoolInit.h>
#include <config/objects/Factory.h>
#include <gcov.h>

#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <fsfw/datapoolglob/GlobalDataPool.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>

/* Global instantiations normally done in main.cpp */
/* Initialize Data Pool */
namespace glob {
GlobalDataPool dataPool(datapool::dataPoolInit);
}


namespace sif {
/* Set up output streams */
ServiceInterfaceStream debug("DEBUG");
ServiceInterfaceStream info("INFO");
ServiceInterfaceStream error("ERROR");
ServiceInterfaceStream warning("WARNING");
}

/* Global object manager */
ObjectManagerIF *objectManager;

int main( int argc, char* argv[] ) {
	// global setup
	objectManager = new ObjectManager(Factory::produce);
	objectManager -> initialize();

	// Catch internal function call
	int result = Catch::Session().run( argc, argv );

	// global clean-up
	return result;
}


#endif
