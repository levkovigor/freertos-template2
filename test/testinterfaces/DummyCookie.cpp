/**
 * @file DummyCookie.cpp
 *
 * @date 06.04.2020
 */

#include <test/testinterfaces/DummyCookie.h>

TestCookie::TestCookie(address_t address_): address(address_) {}

TestCookie::~TestCookie() {}

address_t TestCookie::getAddress() const {
	return address;
}
