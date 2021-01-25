/**
 * @file TmStoreFrontend.cpp
 *
 * @date 15.02.2020
 * @author R. Mueller
 */


//todo: remove code below, this is just used to test the EtlMapWrapper
#include <mission/memory/tmstore/EtlMapWrapperBase.h>
#include <mission/memory/tmstore/EtlMapWrapper.h>

class TTT {
	EtlMapWrapper<int, char, 3> *testBase;
public:
	TTT(EtlMapWrapper<int, char, 3> *test) {
		testBase = test;
	}
	void test() {
		testBase->clear();
		testBase->insert(2, 'a');
		testBase->get(2);
		testBase->erase(2);
	}
};




