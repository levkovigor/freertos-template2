#include <fsfw/objectmanager/ObjectManager.h>
#include <Factory.h>

/* will be created in main */
ObjectManagerIF* objectManager;

namespace sif {
ServiceInterfaceStream debug("DEBUG");
ServiceInterfaceStream info("INFO");
ServiceInterfaceStream warning("WARNING");
ServiceInterfaceStream error("ERROR");
}

void initBootloaderLoader() {
	objectManager = new ObjectManager(Factory::produce);
	objectManager->initialize();
}
