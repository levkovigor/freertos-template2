#ifndef SAM9G20_BOARDTEST_ATMELTESTTASK_H_
#define SAM9G20_BOARDTEST_ATMELTESTTASK_H_

#include "OBSWConfig.h"

#include <test/testtasks/TestTask.h>
#include <fsfw/timemanager/Countdown.h>

extern "C" {
#include <board.h>
}

class AtmelTestTask: public TestTask {
public:
    AtmelTestTask(object_id_t object_id);
    virtual ~AtmelTestTask();
private:
    ReturnValue_t performPeriodicAction() override;
    ReturnValue_t performOneShotAction() override;
    ReturnValue_t performActionA() override;
    ReturnValue_t performActionB() override;

    void printDecoderOutput();
    ReturnValue_t performDataSetTesting(uint8_t testMode);

    void performExceptionTest();
    void performSDCardDemo();
#ifdef ISIS_OBC_G20
    void performIobcHammingTest();
    void performIOBCTest();
    void performNorflashTest();
    // it is assumed software version was written by FRAM handler
    void performFRAMTest();
    void performSupervisorTest();
    void performNorFlashTest(bool displayDebugOutput = true);
#endif
    void performHammingTest();
    void printFilesTest();
    void moveFileTest();

    void tinyfatFsTest();
    Countdown countdown;
};

#endif /* SAM9G20_BOARDTEST_ATMELTESTTASK_H_ */
