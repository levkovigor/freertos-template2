#include <fsfw/action/ActionHelper.h>
#include <fsfw/ipc/CommandMessage.h>
#include <unittest/hosted/catch2/catch.hpp>
#include <unittest/hosted/CatchDefinitions.h>
#include <unittest/hosted/CatchMain.h>
#include <unittest/hosted/TestActionHelper.h>

TEST_CASE( "Action Helper" , "[ActionHelper]") {
	ActionHelperOwnerMockBase testDhMock;
	MessageQueueMockBase testMqMock;
	ActionHelper actionHelper = ActionHelper(
			&testDhMock, dynamic_cast<MessageQueueIF*>(&testMqMock));
	CommandMessage actionMessage;
	ActionId_t testActionId = 777;
	std::array <uint8_t, 3> testParams {1, 2, 3};
	store_address_t paramAddress;
	StorageManagerIF *ipcStore = tglob::getIpcStoreHandle();
	ipcStore->addData(&paramAddress, testParams.data(), 3);
	REQUIRE(actionHelper.initialize() == retval::CATCH_OK);

	SECTION ("Simple tests") {
		 ActionMessage::setCommand(&actionMessage, testActionId, paramAddress);
		 CHECK(not testDhMock.executeActionCalled);
		 actionHelper.handleActionMessage(&actionMessage);
		 CHECK(testDhMock.executeActionCalled);
		 // No message is sent if everything is alright.
		 CHECK(not testMqMock.wasMessageSent());
		 store_address_t invalidAddress;
		 ActionMessage::setCommand(&actionMessage, testActionId, invalidAddress);
		 actionHelper.handleActionMessage(&actionMessage);
		 CHECK(testMqMock.wasMessageSent());
	}
}
