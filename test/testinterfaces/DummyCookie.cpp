/**
 * @file DummyCookie.cpp
 *
 * @date 06.04.2020
 */

#include <test/testinterfaces/DummyCookie.h>

DummyCookie::DummyCookie(address_t address_): address(address_) {}

DummyCookie::~DummyCookie() {}

address_t DummyCookie::getAddress() const {
	return address;
}
