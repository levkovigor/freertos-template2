#include "DummyCookie.h"

DummyCookie::DummyCookie(address_t address, size_t maxReplySize):
        address(address), maxReplySize(maxReplySize) {}

DummyCookie::~DummyCookie() {}

address_t DummyCookie::getAddress() const {
	return address;
}

size_t DummyCookie::getMaxReplySize() const {
    return maxReplySize;
}
