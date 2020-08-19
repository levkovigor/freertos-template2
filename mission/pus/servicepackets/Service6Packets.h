#ifndef MISSION_PUS_SERVICEPACKETS_SERVICE6PACKETS_H_
#define MISSION_PUS_SERVICEPACKETS_SERVICE6PACKETS_H_

#include <fsfw/action/ActionMessage.h>
#include <fsfw/container/FixedArrayList.h>
#include <fsfw/objectmanager/SystemObjectIF.h>
#include <fsfw/serialize/SerializeElement.h>
#include <fsfw/serialize/SerialLinkedListAdapter.h>
#include <fsfw/serialize/SerialFixedArrayListAdapter.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>


/**
 * @brief   Subservice 2
 * @ingroup spacepackets
 */
template <size_t MAX_DATA_SIZE>
class LoadToMemoryCommand: public SerialLinkedListAdapter<SerializeIF> { //!< [EXPORT] : [SUBSERVICE] 2
public:
	typedef uint8_t dataBufferType; //!< [EXPORT] : [IGNORE]
	typedef uint16_t typeOfMaxData; //!< [EXPORT] : [IGNORE]

	LoadToMemoryCommand() {
		address.setNext(&data);
		data.setNext(&checksum);
	}

	uint32_t getAddress() const {
		return address.entry;
	}

	const uint8_t * getData() {
		return data.entry.front();
	}

	typeOfMaxData getDataSize() const {
		return data.entry.size;
	}

	uint16_t getCrcChecksum() const {
		return checksum.entry;
	}

private:
	SerializeElement<uint32_t> address;
	SerializeElement<SerialFixedArrayListAdapter<dataBufferType, MAX_DATA_SIZE,typeOfMaxData>> data; //!< [EXPORT] : [SIZEFIELD] uint16_t [BUFFERTYPE] uint8_t
	SerializeElement<uint16_t> checksum;
};


/**
 * @brief   Subservice 5
 * @ingroup spacepackets
 */
class DumpMemoryCommand: public SerialLinkedListAdapter<SerializeIF> { //!< [EXPORT] : [SUBSERVICE] 5
public:
	DumpMemoryCommand(){
		setStart(&address);
		address.setNext(&dumpLength);
	}

	uint32_t getAddress() const {
		return address.entry;
	}

	uint16_t getDumpLength() {
		return dumpLength.entry;
	}

private:
	SerializeElement<uint32_t> address;
	SerializeElement<uint32_t> dumpLength;
};


/**
 * @brief   Subservice 9
 * @ingroup spacepackets
 */
class CheckMemoryCommand: public SerialLinkedListAdapter<SerializeIF> { //!< [EXPORT] : [SUBSERVICE] 9
public:
	CheckMemoryCommand(){
		setStart(&address);
		address.setNext(&checkLength);
	}

	uint32_t getAddress() const {
		return address.entry;
	}

	uint16_t getCheckLength() {
		return checkLength.entry;
	}

private:
	SerializeElement<uint32_t> address;
	SerializeElement<uint32_t> checkLength;
};


/**
 * @brief   Subservice 6
 * @ingroup spacepackets
 */
class MemoryDumpPacket : public SerializeIF { //!< [EXPORT] : [SUBSERVICE] 6
public:

	MemoryDumpPacket(object_id_t memoryId_, uint32_t address_, uint16_t size_,
			const uint8_t *buffer_, uint16_t checksum_) :
			memoryId(memoryId_), address(address_), length(size_),
			buffer(buffer_), crc(checksum_) {}

	virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size,
	        size_t maxSize, SerializeIF::Endianness streamEndianness
	        ) const override {
		ReturnValue_t result = SerializeAdapter::serialize(&memoryId,buffer,
				size,maxSize, streamEndianness);
		if(result != HasReturnvaluesIF::RETURN_OK){
			return result;
		}
		result = SerializeAdapter::serialize(&address,buffer,
		        size, maxSize , streamEndianness);
		if(result != HasReturnvaluesIF::RETURN_OK){
			return result;
		}
		result = SerializeAdapter::serialize(&length,buffer,
		        size, maxSize , streamEndianness);
		if(result != HasReturnvaluesIF::RETURN_OK){
			return result;
		}

		if(*size + length >maxSize){
			return SerializeIF::BUFFER_TOO_SHORT;
		}
		memcpy(*buffer,this->buffer,this->length);
		(*buffer) += this->length;
		(*size) += this->length;

		result = SerializeAdapter::serialize(&crc,buffer,size,
		        maxSize , streamEndianness);

		return result;

	};

	virtual size_t getSerializedSize() const override {
		uint32_t size = 0;
		size+= SerializeAdapter::getSerializedSize(&memoryId);
		size+= SerializeAdapter::getSerializedSize(&address);
		size+= SerializeAdapter::getSerializedSize(&length);
		size+= length;
		size+= SerializeAdapter::getSerializedSize(&crc);
		return size;
	};

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
	        SerializeIF::Endianness streamEndianness) override {
		return HasReturnvaluesIF::RETURN_FAILED;
	}

private:
	MemoryDumpPacket(const MemoryDumpPacket &command);
	object_id_t memoryId;
	uint32_t address;
	uint16_t length;
	const uint8_t * buffer;
	uint16_t crc;
};

/**
 * @brief   Subservice 10
 * @ingroup spacepackets
 */
class MemoryCheckPacket : public SerialLinkedListAdapter<SerializeIF> { //!< [EXPORT] : [SUBSERVICE] 10
public:
	MemoryCheckPacket() {
		setStart(&address);
		address.setNext(&length);
		length.setNext(&checksum);
	}

	ReturnValue_t setAddress(uint32_t address_) {
		address = address_;
		return HasReturnvaluesIF::RETURN_OK;
	}

	ReturnValue_t setLength(uint32_t length_) {
		length = length_;
		return HasReturnvaluesIF::RETURN_OK;
	}

	ReturnValue_t setChecksum(uint16_t checksum_) {
		checksum = checksum_;
		return HasReturnvaluesIF::RETURN_OK;
	}

	uint32_t getAddress() {
		return address.entry;
	}

	uint32_t getLength() {
		return length.entry;
	}

	uint16_t getChecksum() {
		return checksum.entry;
	}

private:
	SerializeElement<uint32_t> address;
	SerializeElement<uint32_t> length;
	SerializeElement<uint16_t> checksum;
};

#endif /* MISSION_PUS_SERVICEPACKETS_SERVICE6PACKETS_H_ */
