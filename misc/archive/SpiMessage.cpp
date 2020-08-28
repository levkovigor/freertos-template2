/**
 * @file ThermalMessage.cpp
 *
 * @date 16.02.2020
 */

#include <misc/archive/SpiMessage.h>

SpiMessage::SpiMessage() {
}

SpiMessage::~SpiMessage() {
}

void SpiMessage::setRtdValue(CommunicationMessage *message, rtd_t rtdValue) {
	message->setUint32Data(rtdValue);
}

rtd_t SpiMessage::getRtdValue(const CommunicationMessage * message) {
	rtd_t rtdValue = message->getUint32Data();
	return rtdValue;
}

void SpiMessage::clear(CommunicationMessage *message) {
	message->clearCommunicationMessage();
}
