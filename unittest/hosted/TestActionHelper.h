#ifndef UNITTEST_HOSTED_TESTACTIONHELPER_H_
#define UNITTEST_HOSTED_TESTACTIONHELPER_H_

#include <fsfw/action/HasActionsIF.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <unittest/hosted/CatchDefinitions.h>


class ActionHelperOwnerMockBase: public HasActionsIF {
public:
	bool getCommandQueueCalled = false;
	bool executeActionCalled = false;

	MessageQueueId_t getCommandQueue() const override {
		return tconst::testQueueId;
	}

	ReturnValue_t executeAction(ActionId_t actionId, MessageQueueId_t commandedBy,
			const uint8_t* data, size_t size) override {
		executeActionCalled = true;
		return HasReturnvaluesIF::RETURN_OK;
	}
};


class MessageQueueMockBase: public MessageQueueIF {
public:
	MessageQueueId_t myQueueId = 0;
	bool defaultDestSet = false;
	bool messageSent = false;

	bool wasMessageSent() {
		bool tempMessageSent = messageSent;
		messageSent = false;
		return tempMessageSent;
	}

	virtual ReturnValue_t reply( MessageQueueMessageIF* message ) {
		return HasReturnvaluesIF::RETURN_OK;
	};
	virtual ReturnValue_t receiveMessage(MessageQueueMessageIF* message,
			MessageQueueId_t *receivedFrom) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual ReturnValue_t receiveMessage(MessageQueueMessageIF* message) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual ReturnValue_t flush(uint32_t* count) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual MessageQueueId_t getLastPartner() const {
		return tconst::testQueueId;
	}
	virtual MessageQueueId_t getId() const {
		return tconst::testQueueId;
	}
	virtual ReturnValue_t sendMessageFrom( MessageQueueId_t sendTo,
			MessageQueueMessageIF* message, MessageQueueId_t sentFrom,
			bool ignoreFault = false ) {
		messageSent = true;
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual ReturnValue_t sendMessage( MessageQueueId_t sendTo,
			MessageQueueMessageIF* message, bool ignoreFault = false ) override {
		messageSent = true;
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual ReturnValue_t sendToDefaultFrom( MessageQueueMessageIF* message,
			MessageQueueId_t sentFrom, bool ignoreFault = false ) {
		messageSent = true;
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual ReturnValue_t sendToDefault( MessageQueueMessageIF* message ) {
		messageSent = true;
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual void setDefaultDestination(MessageQueueId_t defaultDestination) {
		myQueueId = defaultDestination;
		defaultDestSet = true;
	}

	virtual MessageQueueId_t getDefaultDestination() const {
		return myQueueId;
	}
	virtual bool isDefaultDestinationSet() const {
		return defaultDestSet;
	}
};


#endif /* UNITTEST_TESTFW_NEWTESTS_TESTACTIONHELPER_H_ */
