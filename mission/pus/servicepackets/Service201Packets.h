/**
 * \file Service201Packets.h
 *
 * \brief Structure of a health command or reply
 *
 * \date 16.11.2019
 * \author R. Mueller
 */

#ifndef MISSION_PUS_SERVICEPACKETS_SERVICE201PACKETS_H_
#define MISSION_PUS_SERVICEPACKETS_SERVICE201PACKETS_H_

#include <fsfw/serialize/SerialLinkedListAdapter.h>
#include <fsfw/serialize/SerializeIF.h>
#include <fsfw/health/HasHealthIF.h>

class HealthCommand: public SerialLinkedListAdapter<SerializeIF> { //!< [EXPORT] : [SUBSERVICE] 1
public:
	HealthCommand() {
		setLinks();
	}

	HasHealthIF::HealthState getHealth() {
		return static_cast<HasHealthIF::HealthState>(health.entry);
	}
private:
	// Forbid copying because of next pointer to member
	HealthCommand(const HealthCommand &command);
	void setLinks() {
		setStart(&objectId);
		objectId.setNext(&health);
	}
	SerializeElement<uint32_t> objectId; //!< [EXPORT] : [COMMENT] Target object Id
	SerializeElement<uint8_t> health; //!< [EXPORT] : [COMMENT] Health to set
};


class HealthSetReply: public SerialLinkedListAdapter<SerializeIF> { //!< [EXPORT] : [SUBSERVICE] 2
public:
	HealthSetReply(uint8_t health_, uint8_t oldHealth_):
		health(health_), oldHealth(oldHealth_)
	{
		setLinks();
	}

private:
	HealthSetReply(const HealthSetReply &reply);
	void setLinks() {
		setStart(&objectId);
		objectId.setNext(&health);
		health.setNext(&oldHealth);
	}
	SerializeElement<uint32_t> objectId; //!< [EXPORT] : [COMMENT] Source object ID
	SerializeElement<uint8_t> health; //!< [EXPORT] : [COMMENT] New Health
	SerializeElement<uint8_t> oldHealth; //!< [EXPORT] : [COMMENT] Old Health
};

#endif /* MISSION_PUS_SERVICEPACKETS_SERVICE201PACKETS_H_ */
