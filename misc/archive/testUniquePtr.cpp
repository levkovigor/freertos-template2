// Example program
#include <iostream>
#include <string>
#include <memory>
#include <cstring>

class DummyHelper {
public:
	DummyHelper(uint8_t someResource): someResource(someResource) {
	}

	virtual ~DummyHelper() {
		someResource = 0;
		std::cout << "Dtor called" << std::endl;
	}

	int getResource() {
		return someResource;
	}
private:
	int someResource = 0;
};

class CommandMessage {
public:
	CommandMessage() = default;
	virtual~ CommandMessage() {};

	void setParameter(uint64_t parameter) {
		memcpy(rawContent.data() + 4, &parameter, sizeof(parameter));
	}

	uint64_t getParameter() const {
		uint64_t param;
		memcpy(&param, rawContent.data() + 4, sizeof(param));
		return param;
	}

private:
	std::array<uint8_t, 12> rawContent;
};

void setTestMessage(CommandMessage *testMsg,
		uint8_t someResource) {
	auto unique_pptr = new std::unique_ptr<DummyHelper>(new DummyHelper(someResource));
	uint64_t test_ptr = reinterpret_cast<uint64_t>(unique_pptr);
	testMsg->setParameter(test_ptr);
}

std::unique_ptr<DummyHelper> getUniquePtr(
		CommandMessage *testMsg) {
	uint64_t raw_ptr = testMsg->getParameter();
	auto unique_pptr = reinterpret_cast<std::unique_ptr<DummyHelper> *>(raw_ptr);
	return std::move(*unique_pptr);
}


int main()
{
	auto storagePtr = std::make_unique<DummyHelper>(42);
	CommandMessage testMessage;
	std::cout << "Setting message" << std::endl;
	setTestMessage(&testMessage, 42);
	// a message has been set and can be sent, e.g. via a message queue

	// in another class, the pointer is retrieved.
	auto unique_ptr = getUniquePtr(&testMessage);
	DummyHelper & test = *unique_ptr;
	std::cout << test.getResource() << std::endl;
	unique_ptr.reset();
}
