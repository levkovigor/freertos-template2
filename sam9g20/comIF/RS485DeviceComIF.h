#ifndef SAM9G20_COMIF_RS485DEVICECOMIF_H_
#define SAM9G20_COMIF_RS485DEVICECOMIF_H_

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>

class RS485DeviceComIF: public DeviceCommunicationIF, public SystemObject {
public:
	RS485DeviceComIF(object_id_t object_id_);
	virtual ~RS485DeviceComIF();
};

#endif /* BSP_COMIF_RS485DEVICECOMIF_H_ */
