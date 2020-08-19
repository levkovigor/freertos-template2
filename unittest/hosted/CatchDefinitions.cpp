#include <unittest/hosted/CatchDefinitions.h>

StorageManagerIF* tglob::getIpcStoreHandle() {
	if(objectManager != nullptr) {
		return objectManager->get<StorageManagerIF>(objects::IPC_STORE);
	} else {
		sif::error << "Global object manager uninitialized" << std::endl;
		return nullptr;
	}
}
