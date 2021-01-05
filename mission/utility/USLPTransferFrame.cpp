/**
 * @file	USLPTransferFrame.cpp
 * @brief	This file defines the USLP TransferFrame class.
 * @date	14.12.2020
 * @author	L. Rajer
 */



#include "../../mission/utility/USLPTransferFrame.h"

//#include "../serviceinterface/ServiceInterfaceStream.h"

USLPTransferFrame::USLPTransferFrame() {
	 frame = NULL;
}

USLPTransferFrame::USLPTransferFrame(uint8_t* setData, uint16_t dataFieldSize) {
	 this->frame = (uslp_transfer_frame*)setData;
	 this->dataZoneSize = dataFieldSize;
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






//uint16_t USLPTransferFrame::getFrameLength() {
//	 return  ( (this->frame->primaryHeader.tfvnAndScid & 0b00000011) << 8 ) + this->frame->primaryHeader.tfvnAndScid;
//}
//
//uint16_t USLPTransferFrame::getDataLength() {
//	 return  this->getFrameLength() - this->getHeaderSize() -1 - FRAME_CRC_SIZE + 1; // -1 for the segment primaryHeader.
//}
//
//uint8_t USLPTransferFrame::getSequenceNumber() {
//	 return this->frame->primaryHeader.sequenceNumber;
//}
//
//uint8_t USLPTransferFrame::getSequenceFlags() {
//	 return (this->frame->dataField & 0b11000000)>>6;
//}
//
//
//uint8_t* USLPTransferFrame::getDataField() {
//	return &(this->frame->dataField) + 1;
//}
//
//uint8_t* USLPTransferFrame::getFullFrame() {
//	return (uint8_t*)this->frame;
//}
//
//uint16_t USLPTransferFrame::getFullSize() {
//	return this->getFrameLength() + 1;
//}
//
//uint16_t USLPTransferFrame::getHeaderSize() {
//	 return sizeof(frame->primaryHeader);
//}
//
//uint16_t USLPTransferFrame::getFullDataLength() {
//	return this->getFrameLength() - this->getHeaderSize() - FRAME_CRC_SIZE + 1;
//}
//
//uint8_t* USLPTransferFrame::getFullDataField() {
//	return &frame->dataField;
//}
//
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
