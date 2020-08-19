#ifndef TEST_TESTTASKS_PUSTCINJECTOR_H_
#define TEST_TESTTASKS_PUSTCINJECTOR_H_
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <array>

class PusTcInjector: public SystemObject {
public:
	static constexpr uint8_t INJECTION_QUEUE_DEPTH = 10;
	const uint16_t defaultApid;

	/**
	 * Initialize a software telecommand injector by supplying object IDs to
	 * various helper objects which must exist before calling initialiez()
	 * @param objectId ID of PUS telecommand injector
	 * @param destination ID of destination, which has to implement
	 *  AcceptsTelecommandIF.
	 * @param tcStore ID of telecommand store, which has to implement
	 *  StorageManagerIF.
	 * @param defaultApid Default APID which will be used if an injection
	 *  without an APID is requested.
	 */
	PusTcInjector(object_id_t objectId, object_id_t destination,
			object_id_t tcStore, uint16_t defaultApid);
	/**
	 * This has to be called before using the PusTcInjector.
	 * Call Not necessary when using a factory and the object manager.
	 * @return -@c RETURN_OK for successfull init
	 * 		   -@c ObjectManagerIF::CHILD_INIT_FAILED otherwise
	 */
	ReturnValue_t initialize() override;

	virtual~ PusTcInjector();

	/**
	 * Can be used to inject a telecommand by supplying service, subservice
	 * and optional application data and its length.
	 * Default APID will be used.
	 * @param service PUS service type
	 * @param subservice PUS subservice type
	 * @param appData Pointer to application data
	 * @param appDataLen Length of application data
	 * @return
	 */
	ReturnValue_t injectPusTelecommand(uint8_t service, uint8_t subservice,
			const uint8_t* appData = nullptr, size_t appDataLen = 0);
	/**
	 * Provides the same functionality while also setting a user defined APID.
	 * @param service PUS service type
	 * @param subservice PUS subservice type
	 * @param apid Custom APID to,
	 * @param appData Pointer to application data
	 * @param appDataLen Length of application data
	 * @return
	 */
	ReturnValue_t injectPusTelecommand(uint8_t service, uint8_t subservice,
			uint16_t apid, const uint8_t* appData = nullptr,
			size_t appDataLen = 0);
private:
	MessageQueueIF* injectionQueue = nullptr;
	StorageManagerIF *tcStore = nullptr;

	/* Cached for initialize function */
	object_id_t destination = 0;
	object_id_t tcStoreId = 0;

	uint16_t sequenceCount = 0;
};

#endif /* TEST_TESTTASKS_PUSTCINJECTOR_H_ */
