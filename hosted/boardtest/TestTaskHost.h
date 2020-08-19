#ifndef TEST_TESTTASK_LINUX_H_
#define TEST_TESTTASK_LINUX_H_

#include <test/testtasks/TestTask.h>

#include <vector>

/**
 * @brief 	Simple test class.
 */
class TestTaskHost : public TestTask {
public:
	TestTaskHost(object_id_t object_id);
	virtual ~TestTaskHost();
	ReturnValue_t performOperation(uint8_t operationCode = 0) override;
	ReturnValue_t performOneShotAction() override;
	ReturnValue_t performPeriodicAction() override;

private:
	void testBaseAsFriend();
	void sharedPtrPoolTest();
};

class Base {
	friend class Child;
public:
	Base() = default;
	virtual~ Base() {}
	void basePublicInterface() {
		std::cout << "Hello, I'm the public base interface" << std::endl;
	}

private:
	void baseInternalFunction() {
		std::cout << "Hello, I'm the private base internal method" << std::endl;
	}
};

class Child: public Base {
public:
	Child() = default;
	virtual~ Child() {}
	void childPublicInterface() {
		std::cout << "Hello, I'm the child base interface" << std::endl;
		basePublicInterface();
	}

	void hackToAccessBasePrivate() {
		baseInternalFunction();
	}

private:
	void childInternalFunction() {
		std::cout << "Hello, I'm the child internal method" << std::endl;
	}
};

#endif /* TestTaskLinux_H_ */
