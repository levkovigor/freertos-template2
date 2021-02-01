/**
 * @file	USLPTransferFrame.cpp
 * @brief	This file defines the USLP TransferFrame class.
 * @date	14.12.2020
 * @author	L. Rajer
 */

#include "USLPTransferFrame.h"

USLPTransferFrame::USLPTransferFrame() {
    this->frame = NULL;
    this->dataZoneSize = 0;
}

USLPTransferFrame::USLPTransferFrame(uint8_t *setData, uint16_t dataZoneSize) {
    this->frame = (uslp_transfer_frame*) setData;
    this->dataZoneSize = dataZoneSize;
}

uint8_t USLPTransferFrame::getVersionNumber() {
    return (this->frame->primaryHeader.tfvnAndScid & 0b11110000) >> 4;
}
void USLPTransferFrame::setVersionNumber(uint8_t versionNumber) {
    this->frame->primaryHeader.tfvnAndScid = ((versionNumber & 0b00001111) << 4)
            | (this->frame->primaryHeader.tfvnAndScid & 0b00001111);
}

uint16_t USLPTransferFrame::getSpacecraftId() {
    return ((this->frame->primaryHeader.tfvnAndScid & 0b00001111) << 12)
            + (this->frame->primaryHeader.scid_m << 4)
            + ((this->frame->primaryHeader.scidAndsourceflagAndVcid & 0b11110000) >> 4);
}
void USLPTransferFrame::setSpacecraftId(uint16_t spacecraftID) {
    this->frame->primaryHeader.tfvnAndScid = (spacecraftID >> 12)
            | (this->frame->primaryHeader.tfvnAndScid & 0b11110000);
    this->frame->primaryHeader.scid_m = ((spacecraftID & 0b0000111111110000) >> 4)
            | (this->frame->primaryHeader.scid_m & 0b00000000);
    this->frame->primaryHeader.scidAndsourceflagAndVcid = ((spacecraftID & 0b0000000000001111) << 4)
            | (this->frame->primaryHeader.scidAndsourceflagAndVcid & 0b00001111);
}

bool USLPTransferFrame::sourceFlagSet() {
    return (this->frame->primaryHeader.scidAndsourceflagAndVcid & 0b00001000) != 0;
}
void USLPTransferFrame::setSourceFlag(bool isSet) {
    // Could probably just cast the bool, but this seems like the more "correct" way
    if (isSet) {
        this->frame->primaryHeader.scidAndsourceflagAndVcid |= 0b00001000;
    } else {
        this->frame->primaryHeader.scidAndsourceflagAndVcid &= (~0b00001000);
    }
}

uint8_t USLPTransferFrame::getVirtualChannelId() {
    return ((this->frame->primaryHeader.scidAndsourceflagAndVcid & 0b00000111) << 3)
            + ((this->frame->primaryHeader.vcidAndMapidAndtruncatedflag & 0b11100000) >> 5);
}
void USLPTransferFrame::setVirtualChannelId(uint8_t virtualChannelId) {
    this->frame->primaryHeader.scidAndsourceflagAndVcid = ((virtualChannelId & 0b00111000) >> 3)
            | (this->frame->primaryHeader.scidAndsourceflagAndVcid & 0b11111000);
    this->frame->primaryHeader.vcidAndMapidAndtruncatedflag = ((virtualChannelId & 0b00000111) << 5)
            | (this->frame->primaryHeader.vcidAndMapidAndtruncatedflag & 0b00011111);
}

uint8_t USLPTransferFrame::getMapId() {
    return (this->frame->primaryHeader.vcidAndMapidAndtruncatedflag & 0b00011110) >> 1;
}
void USLPTransferFrame::setMapId(uint8_t mapId) {
    this->frame->primaryHeader.vcidAndMapidAndtruncatedflag = ((mapId & 0b00001111) << 1)
            | (this->frame->primaryHeader.vcidAndMapidAndtruncatedflag & 0b11100001);
}

bool USLPTransferFrame::truncatedFlagSet() {
    return (this->frame->primaryHeader.vcidAndMapidAndtruncatedflag & 0b00000001) != 0;
}
void USLPTransferFrame::setTruncatedFlag(bool isSet) {
    if (isSet) {
        this->frame->primaryHeader.vcidAndMapidAndtruncatedflag |= 0b00000001;
    } else {
        this->frame->primaryHeader.vcidAndMapidAndtruncatedflag &= (~0b00000001);
    }
}

uint8_t USLPTransferFrame::getTFDZConstructionRules() {
    return (this->frame->dataFieldHeader.rulesAndprotocolid & 0b11100000) >> 5;
}
void USLPTransferFrame::setTFDZConstructionRules(uint8_t constructionRules) {
    this->frame->dataFieldHeader.rulesAndprotocolid = ((constructionRules & 0b00000111) << 5)
            | (this->frame->dataFieldHeader.rulesAndprotocolid & 0b00011111);
}

uint8_t USLPTransferFrame::getProtocolIdentifier() {
    return (this->frame->dataFieldHeader.rulesAndprotocolid & 0b00011111);
}
void USLPTransferFrame::setProtocolIdentifier(uint8_t protocolId) {
    this->frame->dataFieldHeader.rulesAndprotocolid = (protocolId & 0b00011111)
            | (this->frame->dataFieldHeader.rulesAndprotocolid & 0b11100000);
}

uint16_t USLPTransferFrame::getFirstHeaderOffset() {
    return this->frame->dataFieldHeader.firstHeaderPointer;
}
void USLPTransferFrame::setFirstHeaderOffset(uint16_t offset) {
    this->frame->dataFieldHeader.firstHeaderPointer = offset;
}

uint8_t* USLPTransferFrame::getFirstHeader() {
    return this->getDataZone() + this->getFirstHeaderOffset();
}

uint16_t USLPTransferFrame::getFullFrameSize() {
    return this->dataZoneSize + FRAME_OVERHEAD;
}

uint16_t USLPTransferFrame::getDataZoneSize() {
    return this->dataZoneSize;
}

uint8_t* USLPTransferFrame::getDataZone() {
    // I have honestly no idea why we have to do this, but if we don't, memcpy copies to the
    // 9th byte of the frame instead of to the 8th like it is supposed to
    // TODO: Pointer gods enlighten me
    return &(this->frame->dataZone) - 1;
}

uint8_t* USLPTransferFrame::getFullFrame() {
    return (uint8_t*) this->frame;
}

//void USLPTransferFrame::print() {
//	sif::debug << "Raw Frame: " << std::hex << std::endl;
//	for (uint16_t count = 0; count < this->getFullSize(); count++ ) {
//		sif::debug << (uint16_t)this->getFullFrame()[count] << " ";
//	}
//	sif::debug << std::dec << std::endl;
//	debug << "Frame Header:" << std::endl;
//	debug << "Version Number: " << std::hex << (uint16_t)this->current_frame.getVersionNumber() << std::endl;
//	debug << "Bypass Flag set?| Ctrl Cmd Flag set?: " << (uint16_t)this->current_frame.bypassFlagSet() << " | " << (uint16_t)this->current_frame.controlCommandFlagSet()  << std::endl;
//	debug << "SCID : " << this->current_frame.getSpacecraftId() << std::endl;
//	debug << "VCID : " << (uint16_t)this->current_frame.getVirtualChannelId() << std::endl;
//	debug << "Frame length: " << std::dec << this->current_frame.getFrameLength() << std::endl;
//	debug << "Sequence Number: " << (uint16_t)this->current_frame.getSequenceNumber() << std::endl;
//}
