#include <CatchDefinitions.h>

#include <config/cdatapool/dataPoolInit.h>
#include <config/objects/Factory.h>

#ifdef GCOV
#include <gcov.h>
#endif

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

int customSetup() {
    // global setup
    objectManager = new ObjectManager(Factory::produce);
    objectManager -> initialize();
    return 0;
}

