/**
 * @file    RS485TmTcTarget.h
 * @brief   This file defines the RS485TmTcTarget class.
 * @date    22.12.2020
 * @author  L. Rajer
 */
#include "RS485TmTcTarget.h"

RS485TmTcTarget::RS485TmTcTarget(object_id_t objectId):SystemObject(objectId)
{

}

RS485TmTcTarget::~RS485TmTcTarget() {}

MessageQueueId_t RS485TmTcTarget::getReportReceptionQueue(uint8_t virtualChannel){
    return tmTcReceptionQueue->getId();
}

ReturnValue_t RS485TmTcTarget::fillSendFrameBuffer(){
    //TODO: Add what name says
    return HasReturnvaluesIF::RETURN_FAILED;
}
