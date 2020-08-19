#include <test/prototypes/TestAutoDeleteMessage.h>
#include <cstring>
#include <memory>

TestAutoDeleteMessage::~TestAutoDeleteMessage() {
}

TestAutoDeleteMessage::TestAutoDeleteMessage() {
}

// Functions for 64bit architectures.
#ifdef IS64BIT

void TestAutoDeleteMessage::setTestMessage(
		CommandMessage *testMsg,
		store_address_t storeId, StorageManagerIF * pool) {
	auto unique_pptr = new std::unique_ptr<StorageHelper>(
			new StorageHelper(storeId, pool));
	uint64_t testPtr = reinterpret_cast<uint64_t>(unique_pptr);
	testMsg->setCommand(UNIQUE_PTR_TEST_CMD);
	memcpy(testMsg->getBuffer() + 4, &testPtr, sizeof(void*));
}

std::unique_ptr<StorageHelper> TestAutoDeleteMessage::getUniquePtr(
		CommandMessage *testMsg) {
	uint64_t raw_ptr;
	memcpy(&raw_ptr, testMsg->getBuffer() + 4, sizeof(void*));
	auto unique_pptr = reinterpret_cast<std::unique_ptr<StorageHelper> *>(raw_ptr);
	auto unique_ptr = std::move(*unique_pptr);
	delete unique_pptr;
	return unique_ptr;
}

#else

void TestAutoDeleteMessage::setTestMessage(CommandMessage *testMsg,
		store_address_t storeId, StorageManagerIF * pool) {
	auto unique_pptr = new std::unique_ptr<StorageHelper>(
			new StorageHelper(storeId, pool));
	testMsg->setCommand(UNIQUE_PTR_TEST_CMD);
	uint32_t testPtr = reinterpret_cast<uint32_t>(unique_pptr);
	testMsg->setParameter(testPtr);
}


std::unique_ptr<StorageHelper> TestAutoDeleteMessage::getUniquePtr(
		CommandMessage *testMsg) {
	uint32_t raw_ptr = testMsg->getParameter();
	auto unique_pptr = reinterpret_cast<std::unique_ptr<StorageHelper> *>(raw_ptr);
	auto unique_ptr = std::move(*unique_pptr);
	delete unique_pptr;
	return unique_ptr;
}

#endif

