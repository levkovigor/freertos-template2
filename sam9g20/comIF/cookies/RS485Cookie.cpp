#include "RS485Cookie.h"



RS485Cookie::RS485Cookie(){}

RS485Cookie::~RS485Cookie(){}

void RS485Cookie::setDevice(RS485Devices device){
	this->device = device;
}

RS485Devices RS485Cookie::getDevice() const{
	return device;
}


