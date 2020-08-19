#ifndef TEST_TESTAUTODELETEMESSAGE_H_
#define TEST_TESTAUTODELETEMESSAGE_H_
#include <config/ipc/MissionMessageTypes.h>
#include <fsfw/ipc/CommandMessage.h>
#include <test/prototypes/StorageHelper.h>
#include <memory>
#include <cstring>
#include <cstdint>

#if INTPTR_MAX == INT32_MAX
#define IS32BIT
#else
#define IS64BIT
#endif

class TestAutoDeleteMessage: public CommandMessage {
public:
	virtual ~TestAutoDeleteMessage();
	static constexpr uint32_t MESSAGE_ID = MESSAGE_TYPE::TEST_UNIQUE_PTR;
	static constexpr Command_t UNIQUE_PTR_TEST_CMD = MAKE_COMMAND_ID( 0 );


	static void setTestMessage(CommandMessage *testMsg,
			store_address_t storeId, StorageManagerIF * pool);
	static std::unique_ptr<StorageHelper> getUniquePtr(CommandMessage *testMsg);
private:
	TestAutoDeleteMessage();

};

#endif /* TEST_TESTAUTODELETEMESSAGE_H_ */
