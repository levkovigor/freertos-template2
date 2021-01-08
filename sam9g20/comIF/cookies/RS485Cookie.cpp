#include "RS485Cookie.h"



RS485Cookie::RS485Cookie(RS485Devices device){
	this->device = device;
	sendLen = 0;
	writeData = nullptr;
}

RS485Cookie::~RS485Cookie(){}

void RS485Cookie::setDevice(RS485Devices device){
	this->device = device;
}

RS485Devices RS485Cookie::getDevice() const{
	return device;
}

unsigned int RS485Cookie::getSendLen(){
	return sendLen;
}
void RS485Cookie::setSendLen(unsigned int sendLen){
	this->sendLen = sendLen;
}

unsigned char* RS485Cookie::getWriteData(){
	return writeData;
}
void RS485Cookie::setWriteData(unsigned char* writeData){
	this->writeData = writeData;
}

ComStatusRS485 RS485Cookie::getComStatus(){
	return comStatus;
}
void RS485Cookie::setComStatus(ComStatusRS485 status){
	this->comStatus = status;
}

int8_t RS485Cookie::getReturnValue(){
	return returnValue;
}
void RS485Cookie::setReturnValue(int8_t retval){
	this->returnValue = retval;
}

