/**
 * @file TmStoreFrontend.cpp
 *
 * @date 15.02.2020
 * @author R. Mueller
 */


//todo: remove code below, this is just used to test the EtlMapWrapper
#include <mission/memory/tmstore/EtlMapWrapperIF.h>
#include <mission/memory/tmstore/EtlMapWrapper.h>
#include <mission/memory/tmstore/EtlMultiMapWrapperIF.h>
#include <mission/memory/tmstore/EtlMultiMapWrapper.h>

class TTT {
	EtlMapWrapper<int, char, 3> *testBase;
	EtlMultiMapWrapper<int, char, 10> *multiTestBase;
public:
	TTT(EtlMapWrapper<int, char, 3> *test, EtlMultiMapWrapper<int, char, 10> *test2) {
		testBase = test;
		multiTestBase = test2;
	}
	void test() {
		testBase->clear();
		testBase->insert(2, 'a');
		testBase->get(2);
		testBase->erase(2);
		testBase->insert(2, 'a');
		testBase->eraseByValue('a');


		multiTestBase->clear();
		multiTestBase->insert(2, 'a');
		multiTestBase->insert(2, 'b');
		multiTestBase->get(2);
		multiTestBase->erase(2, 'a');
		multiTestBase->insert(2, 'a');

	}
};




