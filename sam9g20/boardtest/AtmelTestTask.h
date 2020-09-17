#ifndef SAM9G20_BOARDTEST_ATMELTESTTASK_H_
#define SAM9G20_BOARDTEST_ATMELTESTTASK_H_

#include <test/testtasks/TestTask.h>
#include <config/cdatapool/dataPoolInit.h>
#include <config/cdatapool/testPool.h>
#include <fsfw/datapoolglob/GlobalDataSet.h>
#include <fsfw/datapoolglob/GlobalPoolVariable.h>
#include <fsfw/datapoolglob/GlobalPoolVector.h>

extern "C" {
#include <board.h>
}

class TestDataSet: public GlobDataSet {
public:
    TestDataSet(TestInit::TestIdStruct testStruct);
    gp_bool_t testBool;
    gp_uint8_t testUint8;
    gp_uint16_t testUint16;
    gp_uint32_t testUint32;
    gp_vec_t<float,2> testFloatVector;
};

class AtmelTestTask: public TestTask {
public:
    AtmelTestTask(object_id_t object_id, TestInit::TestIdStruct id_struct);
    virtual ~AtmelTestTask();
private:
    ReturnValue_t performPeriodicAction() override;
    ReturnValue_t performOneShotAction() override;
    ReturnValue_t performActionA() override;
    ReturnValue_t performActionB() override;

    void printDecoderOutput();
    ReturnValue_t performDataSetTesting(uint8_t testMode);
    void performNewPoolManagerAccessTests();
    void performRunTimeStatsTesting();
    TestDataSet testDataSet;

    void performExceptionTest();
    void performSDCardDemo();
#ifdef ISIS_OBC_G20
    void performIOBCTest();
    void performNorflashTest();
    void performSupervisorTest();
    void performNorFlashTest(bool displayDebugOutput = true);
#endif
};

#endif /* SAM9G20_BOARDTEST_ATMELTESTTASK_H_ */
