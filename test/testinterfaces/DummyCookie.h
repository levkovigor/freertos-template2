#ifndef DUMMYCOOKIE_H_
#define DUMMYCOOKIE_H_

#include <fsfw/devicehandlers/CookieIF.h>
#include <cstddef>

/**
 * Use this to identify different device handlers for DummyComIF
 */
class DummyCookie : public CookieIF {
public:
	DummyCookie(address_t address, size_t maxReplySize);
	virtual ~DummyCookie();

	size_t getMaxReplySize() const;
	address_t getAddress() const;
private:
	address_t address = 0;
	size_t maxReplySize = 0;
};

#endif /* RMAPCOOKIE_H_ */
