#ifndef CONFIG_OBJECTS_SYSTEMOBJECTLIST_H_
#define CONFIG_OBJECTS_SYSTEMOBJECTLIST_H_

#include <cstdint>

// The objects will be instantiated in the ID order
namespace objects {
	enum sourceObjects: uint32_t {
		SERIAL_RECEIVER_TASK  = 1
	};
}

#endif /* BSP_CONFIG_OBJECTS_SYSTEMOBJECTLIST_H_ */
