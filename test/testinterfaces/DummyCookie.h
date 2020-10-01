/**
 * @file DummyCookie.cpp
 *
 * @date 06.04.2020
 */

#ifndef DUMMYCOOKIE_H_
#define DUMMYCOOKIE_H_

#include <fsfw/devicehandlers/CookieIF.h>

/**
 * Use this to identify different device handlers for DummyComIF
 */
class TestCookie : public CookieIF {
public:
	TestCookie(address_t address_);
	virtual ~TestCookie();

	address_t getAddress() const;
private:
	address_t address;
};

#endif /* RMAPCOOKIE_H_ */
