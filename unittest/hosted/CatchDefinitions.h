#pragma once

#include <fsfw/ipc/MessageQueueSenderIF.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/storagemanager/StorageManagerIF.h>

namespace retval {
static constexpr int CATCH_OK = static_cast<int>(HasReturnvaluesIF::RETURN_OK);
static constexpr int CATCH_FAILED = static_cast<int>(HasReturnvaluesIF::RETURN_FAILED);
}

namespace tconst {
	static constexpr MessageQueueId_t testQueueId = 42;
}

namespace tglob {
	StorageManagerIF* getIpcStoreHandle();
}

