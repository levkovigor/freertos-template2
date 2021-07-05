#ifndef TEST_TESTTASK_H_
#define TEST_TESTTASK_H_

#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/parameters/HasParametersIF.h>
#include <fsfw/serialize/SerialBufferAdapter.h>
#include <fsfw/serialize/SerializeElement.h>
#include <fsfw/serialize/SerialLinkedListAdapter.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>

#include <vector>
#include <memory>
#include <cstddef>
#include <etl/map.h>
#include <map>

/**
 * @brief 	Test class for general C++ testing.
 * @details
 * Should not be used for board specific
 * tests. Instead, a derived board test class should be used.
 */
class TestTask :
        public SystemObject,
        public ExecutableObjectIF,
        public HasReturnvaluesIF {
public:
    TestTask(object_id_t objectId);
    virtual ~TestTask();
    virtual ReturnValue_t performOperation(uint8_t operationCode = 0);

protected:
    static constexpr uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::TEST_TASK;

    static constexpr Event TEST_EVENT = MAKE_EVENT(0, severity::INFO);
    static constexpr Event TEST_EVENT_HIGH = MAKE_EVENT(1, severity::HIGH);

    virtual ReturnValue_t performOneShotAction();
    virtual ReturnValue_t performPeriodicAction();
    virtual ReturnValue_t performActionA();
    virtual ReturnValue_t performActionB();

    enum testModes: uint8_t {
        A,
        B
    };

    testModes testMode;

    bool testFlag = false;
    uint8_t counter { 1 };
    uint8_t counterTrigger { 3 };

    void performPusInjectorTest();
    void examplePacketTest();
private:
    // Actually, to be really thread-safe, a mutex should be used as well
    // Let's keep it simple for now.
    static bool oneShotAction;
    static MutexIF* testLock;
    StorageManagerIF* IPCStore;

    void performEtlTemplateTest();

    // void sdTest();

    struct TmManagerStructBase {
        virtual~TmManagerStructBase() = default;
    };

    template<size_t SIZE>
    struct TmManagerStruct: public TmManagerStructBase {
        etl::map<int, int, SIZE> testMap;
    };

    std::map<uint32_t, struct TmManagerStructBase*> testMap;
    static constexpr std::array<size_t, 5> templateSizes = {30, 0, 0, 0, 0};

    template<size_t size>
    void insertNewTmManagerStruct(const uint32_t poolId);
};

template<size_t size>
inline void TestTask::insertNewTmManagerStruct(const uint32_t poolId) {
    // has to be a constant, e.g. a #define or a static constexpr of the class.
    testMap.emplace(poolId, new TmManagerStruct<size>);
}

class TestDummyClass {
public:
    TestDummyClass() = default;
    virtual ~TestDummyClass() {};
private:
    uint8_t dummyMember = 0;
};


class TestExamplePacket: public SerialLinkedListAdapter<SerializeIF> {
public:
    /**
     * For Deserialization
     */
    TestExamplePacket(const uint8_t *somePacket, size_t size, uint8_t * storePointer,
            size_t storeSize):
                buffer(storePointer, storeSize)
    {
        setLinks();
    }

    /**
     * For Serialization
     */
    TestExamplePacket(uint32_t header, uint32_t tail,
            const uint8_t* parameters, size_t paramSize):
                header(header), buffer(parameters, paramSize),
                tail(tail) {
        setLinks();
    }

    uint32_t getHeader() const {
        return header.entry;
    }

    const uint8_t * getBuffer() {
        return buffer.entry.getConstBuffer();
    }

    size_t getBufferLength() {
        return buffer.getSerializedSize();
    }


    uint16_t getTail() const {
        return tail.entry;
    }

private:
    void setLinks() {
        setStart(&header);
        header.setNext(&buffer);
        buffer.setNext(&tail);
        tail.setEnd();
    }

    SerializeElement<uint32_t> header = 0;
    SerializeElement<SerialBufferAdapter<uint8_t>> buffer;
    SerializeElement<uint32_t> tail = 0;
};

#endif /* TESTTASK_H_ */
