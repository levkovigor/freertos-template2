#include "RS485Cookie.h"

RS485Cookie::RS485Cookie(RS485Timeslot timeslot, RS485BaudRates baudrate,
        uint8_t uslp_virtual_channel_id, size_t uslp_tfdz_size, uint8_t uslp_deviceCom_map_id,
        bool hasTmTc, uint8_t uslp_tmTc_map_id) :
        timeslot(timeslot), baudrate(baudrate), uslp_virtual_channel_id(uslp_virtual_channel_id), uslp_tfdz_size(
                uslp_tfdz_size), uslp_deviceCom_map_id(uslp_deviceCom_map_id), hasTmTc(hasTmTc), uslp_tmTc_map_id(
                uslp_tmTc_map_id) {
}

RS485Cookie::~RS485Cookie() {
}

void RS485Cookie::setTimeslot(RS485Timeslot timeslot) {
    this->timeslot = timeslot;
}

RS485Timeslot RS485Cookie::getTimeslot() const {
    return timeslot;
}

ComStatusRS485 RS485Cookie::getComStatus() {
    return comStatus;
}
void RS485Cookie::setComStatus(ComStatusRS485 status) {
    this->comStatus = status;
}

int8_t RS485Cookie::getReturnValue() const {
    return returnValue;
}
void RS485Cookie::setReturnValue(int8_t retval) {
    this->returnValue = retval;
}

uint32_t RS485Cookie::getBaudrate()const {
    return baudrate;
}

uint8_t RS485Cookie::getVcId() const{
    return uslp_virtual_channel_id;
}

uint8_t RS485Cookie::getDevicComMapId() const{
    return uslp_deviceCom_map_id;
}

uint8_t RS485Cookie::getTmTcMapId()const {
    return uslp_tmTc_map_id;
}

size_t RS485Cookie::getTfdzSize()const {
    return uslp_tfdz_size;
}

bool RS485Cookie::getHasTmTc() const {
    return hasTmTc;
}
