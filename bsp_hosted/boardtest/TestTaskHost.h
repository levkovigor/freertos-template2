#ifndef TEST_TESTTASK_LINUX_H_
#define TEST_TESTTASK_LINUX_H_

#include <test/testtasks/TestTask.h>

#include <vector>

/**
 * @brief 	Simple test class.
 */
class TestTaskHost : public TestTask {
public:
	TestTaskHost(object_id_t object_id, bool periodicPrint = false);
	virtual ~TestTaskHost();
	ReturnValue_t performOperation(uint8_t operationCode = 0) override;
	ReturnValue_t performOneShotAction() override;
	ReturnValue_t performPeriodicAction() override;

private:
	void testBaseAsFriend();
	void sharedPtrPoolTest();

	bool periodicPrint = false;
};

class Base {
	friend class Child;
public:
	Base() = default;
	virtual~ Base() {}
	void basePublicInterface() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		std::cout << "Hello, I'm the public base interface" << std::endl;
#endif
	}

private:
	void baseInternalFunction() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		std::cout << "Hello, I'm the private base internal method" << std::endl;
#endif
	}
};

class Child: public Base {
public:
	Child() = default;
	virtual~ Child() {}
	void childPublicInterface() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		std::cout << "Hello, I'm the child base interface" << std::endl;
#endif
		basePublicInterface();
	}

	void hackToAccessBasePrivate() {
		baseInternalFunction();
	}

private:
	void childInternalFunction() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		std::cout << "Hello, I'm the child internal method" << std::endl;
#endif
	}
};

#endif /* TestTaskLinux_H_ */
