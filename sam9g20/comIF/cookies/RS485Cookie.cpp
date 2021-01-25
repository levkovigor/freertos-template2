#include "RS485Cookie.h"

RS485Cookie::RS485Cookie(RS485Devices device, RS485BaudRates baudrate) :
        device(device), baudrate(baudrate) {

}

RS485Cookie::~RS485Cookie() {
}

void RS485Cookie::setDevice(RS485Devices device) {
    this->device = device;
}

RS485Devices RS485Cookie::getDevice() const {
    return device;
}

ComStatusRS485 RS485Cookie::getComStatus() {
    return comStatus;
}
void RS485Cookie::setComStatus(ComStatusRS485 status) {
    this->comStatus = status;
}

int8_t RS485Cookie::getReturnValue() {
    return returnValue;
}
void RS485Cookie::setReturnValue(int8_t retval) {
    this->returnValue = retval;
}

uint32_t RS485Cookie::getBaudrate(){
    return baudrate;
}
