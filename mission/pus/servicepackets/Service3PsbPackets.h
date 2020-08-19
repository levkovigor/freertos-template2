#ifndef MISSION_PUS_SERVICEPACKETS_SERVICE3PSBPACKETS_H_
#define MISSION_PUS_SERVICEPACKETS_SERVICE3PSBPACKETS_H_

#include <config/hk/sid.h>

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/serialize/SerialLinkedListAdapter.h>
#include <fsfw/serialize/SerialBufferAdapter.h>
#include <fsfw/serialize/SerializeElement.h>
#include <fsfw/serialize/SerialFixedArrayListAdapter.h>
#include <mission/pus/Service3HousekeepingPSB.h>

/**
 * @brief 	Subservice 25, 26
 * @details Packet containing the housekeeping
 * 			parameter report with the structure ID (SID)
 * 			and the actual parameters
 * @ingroup spacepackets
 */
class HkParameterReport { //!< [EXPORT] : [SUBSERVICE] 25, 26
public:
	sid32_t sid; //!< [EXPORT] : [COMMENT] SID
	const uint8_t* hkBuffer; //!< [EXPORT] : [MAXSIZE] 1024 Bytes

	HkParameterReport(uint32_t sid_, const uint8_t* parameterBuffer_):
		sid(sid_),hkBuffer(parameterBuffer_) {
	}

	sid32_t getSID() const {
		return sid;
	}

	const uint8_t*  getParameterBuffer() {
		return hkBuffer;
	}

private:
};


/**
 * @brief Generic HK report structure also containg all variables
 * 		  needed for internal handling
 * @details
 * This packet is used for internal handling, serialization and deserialization
 * of packets from the HK pool, deserialization of ground packets from the
 * subservice 1 and 2 and serialization for the subservices 10 and 12.
 * @ingroup spacepackets
 */
class HkPacketDefinition : public SerialLinkedListAdapter<SerializeIF> { //!< [EXPORT] : [SUBSERVICE] 1, 2, 10, 12
public:
	/**
	 * @brief 	This constructor is used to deserialize a HK definition buffer.
	 * @details
	 * If the packet is an internal packet from pool, deSerialize
	 * can be called directly.If the packet is coming from ground, the
	 * fromGround flag must be passed to the deSerialize() function.
	 * Please note that the SerialFixedArrayAdapter reads the parameter list
	 * size automatically. If the packet needs to be initialized with objects,
	 * call initialize() after instantiating it.
	 */
	HkPacketDefinition(): parameterList(static_cast<uint8_t *>(nullptr), 0) {
		setInternalStorageLinks();
		packetFormat = packetFormat::HK_POOL;
	}

	void initialize(sid32_t sid_, float collectionInterval_,
			uint8_t numberOfParameters_, uint32_t * parameterList_,
			bool enableHkPacket_, uint32_t cycleCounter_ = 1) {
		setPredefinedInitLinks();
		sid = sid_;
		hkReportingEnabled = enableHkPacket_;
		collectionInterval = collectionInterval_;
		numberOfParameters = numberOfParameters_;
		parameterList.entry.setBuffer(
				reinterpret_cast<uint8_t*>(parameterList_),
				numberOfParameters * 4
		);
		cycleCounter = cycleCounter_;
	}

	enum packetFormat {
		FROM_GROUND,
		HK_POOL,
		TO_GROUND
	};

	uint8_t getPacketFormat() {
		return packetFormat;
	}

	sid32_t getSID() const {
		return sid.entry;
	}

	void setSID(sid32_t sid_) {
		sid.entry = sid_;
	}

	bool reportingEnabled () {
		return hkReportingEnabled.entry;
	}

	void setReportingStatus (bool hkReportFlag) {
		hkReportingEnabled = hkReportFlag;
	}

	uint32_t getCycleCounter() const {
		return cycleCounter.entry;
	}

	void setCycleCounter(uint32_t newCycleCounter) {
		cycleCounter.entry = newCycleCounter;
	}

	float getCollectionInterval() const {
		return collectionInterval.entry;
	}

	void setCollectionInterval(float newCollectionInterval) {
		collectionInterval.entry = newCollectionInterval;
	}

	uint8_t getNumberOfParameters() const {
		return numberOfParameters.entry;
	}

	void setParametersLength(uint8_t newLength) {
		numberOfParameters = newLength;

	}

	uint32_t * getParameters () {
		return reinterpret_cast<uint32_t *>(parameterListDeserialized.entry.front());
	}

	/**
	 * Override default deSerialize to set numberOfParameters and parameter
	 * address. This  function is called for internal extraction from pool
	 * or when deSerializing packets from ground
	 * @param buffer
	 * @param size
	 * @param bigEndian
	 * @param isDiagnostics Only needed for packets from ground.
	 * Specifies calculation of collection interval
	 * @return
	 */
	virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
	        SerializeIF::Endianness streamEndianness, bool fromGround = false) {
		if(fromGround) {
			setGroundDeserializationLinks();
			packetFormat = packetFormat::FROM_GROUND;
		}
		ReturnValue_t result = SerialLinkedListAdapter::deSerialize(buffer,
				size, streamEndianness);
		if(result == HasReturnvaluesIF::RETURN_OK) {
			numberOfParameters = parameterListDeserialized.entry.size;
			parameterList.entry.setBuffer(reinterpret_cast<uint8_t*>(
					parameterListDeserialized.entry.front()),
					numberOfParameters * 4);
		}

		return result;
	}

	/**
	 * Used for subservices 1 and 2 (HK packet definition from ground)
	 * @param sid_
	 */
	void setGroundDeserializationLinks() {
		setStart(&sid);
		sid.setNext(&collectionInterval);
		collectionInterval.setNext(&parameterListDeserialized);
		parameterListDeserialized.setEnd();
	}

	/**
	 * @brief Used to prepare subservice 1 and 2 packet for HK pool after
	 * SID has been extracted
	 */
	void prepareGroundPacketForHkPool(bool enableReporting_ = false,
			uint32_t initCylceCount_ = 1) {
		hkReportingEnabled = enableReporting_;
		cycleCounter = initCylceCount_;
		packetFormat = packetFormat::HK_POOL;
		setInternalStorageLinks();
	}

	/**
	 * @brief Used for subservices 10 and 12 (Structure report)
	 * @param sid_
	 */
	void setStructureReportLinks(sid32_t sid_){
		setSID(sid_);
		setStart(&sid);
		sid.setNext(&hkReportingEnabled);
		hkReportingEnabled.setNext(&collectionInterval);
		collectionInterval.setNext(&parameterListDeserialized);
		parameterListDeserialized.setEnd();
	}

private:
	void setInternalStorageLinks() {
		setStart(&collectionInterval);
		collectionInterval.setNext(&parameterListDeserialized);
		parameterListDeserialized.setNext(&hkReportingEnabled);
		hkReportingEnabled.setNext(&cycleCounter);
	}

	void setPredefinedInitLinks() {
		setStart(&collectionInterval);
		collectionInterval.setNext(&numberOfParameters);
		numberOfParameters.setNext(&parameterList);
		parameterList.setNext(&hkReportingEnabled);
		hkReportingEnabled.setNext(&cycleCounter);
	}

	SerializeElement<sid32_t> sid = 0;//!< [EXPORT] : [COMMENT] ID used to identify housekeeping data structure
	SerializeElement<bool> hkReportingEnabled = false; //!< [EXPORT] : [OPTIONAL] Subservice 10, 12
	SerializeElement<float> collectionInterval = 0.0; //!< [EXPORT] : [COMMENT] Specifies Collection Interval as factor of smallest sampling time (1.0 seconds for HK, 0.2 seconds for diagnostics)
	SerializeElement<uint8_t> numberOfParameters = 0;
	SerializeElement<SerialFixedArrayListAdapter //!< [EXPORT] : [IGNORE]
	<uint32_t,sid::MAX_LARGE_HK_BUFFER_SIZE,uint8_t>> parameterListDeserialized;  //!< [EXPORT] : [IGNORE]
	SerializeElement<SerialBufferAdapter<uint8_t>> parameterList; //!< [EXPORT] : [MAXSIZE] 1024 Bytes [TYPE] uint32_t
	SerializeElement<uint32_t> cycleCounter = 0; //!< [EXPORT] : [IGNORE]

	uint8_t packetFormat; //!< [EXPORT] : [IGNORE]
};


// SHOULDDO: Modify to allow for multiple collection intervals to be modified
// This propably requires a SerialFixedArrayListAdapter with T = uint32_t
// and some way to access it's values in a clean way.
class hkModifyCollectionIntervalCommand: public SerialLinkedListAdapter<SerializeIF> { //!< [EXPORT] : [SUBSERVICE] 31, 32
public:
	hkModifyCollectionIntervalCommand() {
		setLinks();
	}

	typedef uint64_t dataBufferType;
	typedef uint8_t typeOfMaxData;
	// in bytes
	static const typeOfMaxData MAX_DATA_LENGTH = sid::MAX_LARGE_HK_BUFFER_SIZE - 5;

	sid32_t getSID() {
		return sid.entry;
	}

	uint32_t getInterval_seconds() {
		return collectionInterval.entry;
	}

private:

	void setLinks() {
		setStart(&sid);
		sid.setNext(&collectionInterval);
	}

	// SerializeElement<uint8_t> N; //!< [EXPORT] : [COMMENT] Number of collection intervals to change [IGNORE]
	// SerializeElement<SerialFixedArrayListAdapter<dataBufferType, MAX_DATA_LENGTH, typeOfMaxData>> sidCollectionIntervalBuffer; //!< [EXPORT] :  [IGNORE] [SIZEFIELD] uint8_t [BUFFERTYPE] uint64_t [COMMENT] Buffer content 1 target sid (uint32_t), buffer content 2 target collection interval in seconds (float)
	SerializeElement<sid32_t> sid;
	SerializeElement<float> collectionInterval;

};


class hkAppendParametersCommand: public SerialLinkedListAdapter<SerializeIF> { //!< [EXPORT] : [SUBSERVICE] 29, 30
public:
	hkAppendParametersCommand(){
		setLinks();
	}

	uint32_t getSID() {
		return sid.entry;
	}

	const uint32_t * getParameters() {
		return const_cast<const uint32_t *>(parameters.entry.front());
	}

	// TODO: we should check all parameters, but i don't know if i can use
	// data pool exists() for that. for now, don't check and copy the parameter
	// buffer to the end of the existing parameter buffer
	void checkParameters() {}

	uint8_t getNumberOfParameters() {
		return parameters.entry.size;
	}

private:
	void setLinks() {
		setStart(&sid);
		sid.setNext(&parameters);
	}

	SerializeElement<sid32_t> sid = 0; //!< [EXPORT] : [COMMENT] SID
	SerializeElement<
			SerialFixedArrayListAdapter<uint32_t, sizeof(uint8_t), uint8_t> >
			parameters;
};

#endif /* MISSION_PUS_SERVICEPACKETS_SERVICE3PSBPACKETS_H_ */

