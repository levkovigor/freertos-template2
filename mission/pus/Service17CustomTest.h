#ifndef MISSION_PUS_SERVICE17CUSTOMTEST_H_
#define MISSION_PUS_SERVICE17CUSTOMTEST_H_

#include <fsfw/pus/Service17Test.h>
#include <fsfw/objectmanager/SystemObject.h>

/**
 * @brief Test Service
 * Full Documentation: ECSS-E70-41A p.167
 *
 * The test service provides the capability to activate test functions
 * implemented on-board and to report the results of such tests.
 * Service capability:
 *   - TC[17,1]: Perform connection test
 *   - TM[17,2]: Send Connection Test Report
 *   - TC[17,128]: Perform connection test and trigger event
 *
 * @ingroup pus_services
 */
class Service17CustomTest: public Service17Test {
public:
    enum CustomSubservice {
        //! [EXPORT] : [COMMAND] Enable periodic output
        ENABLE_PERIODIC_PRINT = 128,
        //! [EXPORT] : [COMMAND] Disable periodic output
        DISABLE_PERIODIC_PRINT = 129,

        //! [EXPORT] : [COMMAND] Trigger a software exception which should lead
        //! to a restart.
        TRIGGER_EXCEPTION = 150
    };

	Service17CustomTest(object_id_t objectId, uint16_t apid, uint8_t serviceId);
	virtual ~Service17CustomTest();
	virtual ReturnValue_t handleRequest(uint8_t subservice) override;
	virtual ReturnValue_t performService() override;
private:
	bool periodicPrintoutEnabled = false;
	uint8_t counter = 0;
};

#endif /* MISSION_PUS_SERVICE17CUSTOMTEST_H_ */
