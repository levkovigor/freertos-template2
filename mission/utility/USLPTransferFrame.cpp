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

USLPTransferFrame::USLPTransferFrame(uint8_t* setData, uint16_t dataZoneSize) {
	 this->frame = (uslp_transfer_frame*)setData;
	 this->dataZoneSize = dataZoneSize;
}

uint8_t USLPTransferFrame::getVersionNumber() {
	 return (this->frame->primaryHeader.tfvnAndScid & 0b11110000) >> 4;
}

uint16_t USLPTransferFrame::getSpacecraftId() {
	 return  ((this->frame->primaryHeader.tfvnAndScid & 0b00001111) << 12 ) +
			 (this->frame->primaryHeader.scid_m << 4)  +
			 ((this->frame->primaryHeader.scidAndsourceflagAndVcid & 0b11110000) >> 4);
}

bool USLPTransferFrame::sourceFlagSet() {
	 return (this->frame->primaryHeader.scidAndsourceflagAndVcid & 0b00001000) != 0;
}

uint8_t USLPTransferFrame::getVirtualChannelId() {
	 return  ((this->frame->primaryHeader.scidAndsourceflagAndVcid & 0b00000111) << 3) +
			 ((this->frame->primaryHeader.vcidAndMapidAndtruncatedflag & 0b11100000) >> 5);
}

uint8_t USLPTransferFrame::getMAPId() {
	return (this->frame->primaryHeader.vcidAndMapidAndtruncatedflag & 0b00011110) >> 1;
}

bool USLPTransferFrame::truncatedFlagSet() {
	 return (this->frame->primaryHeader.vcidAndMapidAndtruncatedflag & 0b00000001) != 0;
}

uint8_t USLPTransferFrame::getTFDZConstructionRules(){
	return (this->frame->dataFieldHeader.rulesAndprotocolid & 0b11100000) >> 5;
}

uint8_t USLPTransferFrame::getProtocolIdentifier(){
	return (this->frame->dataFieldHeader.rulesAndprotocolid & 0b00011111);
}
uint16_t USLPTransferFrame::getFirstHeaderOffset(){
	return (this->frame->dataFieldHeader.firstHeader_h << 8) +
			this->frame->dataFieldHeader.firstHeader_l;
}
uint8_t* USLPTransferFrame::getFirstHeader(){
	return this->getDataZone() + this->getFirstHeaderOffset();
}
uint16_t USLPTransferFrame::getFullFrameSize(){
	return this->dataZoneSize + FRAME_OVERHEAD;
}
uint16_t USLPTransferFrame::getDataZoneSize(){
	return this->dataZoneSize;
}
uint8_t* USLPTransferFrame::getDataZone(){
	return &frame->dataZone;
}
uint8_t* USLPTransferFrame::getFullFrame(){
	return (uint8_t*)this->frame;
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
