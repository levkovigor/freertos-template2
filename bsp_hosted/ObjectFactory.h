#ifndef FACTORY_H_
#define FACTORY_H_

#include <fsfw/objectmanager/SystemObjectIF.h>

namespace Factory {

/**
 * @brief 	Creates all SystemObject elements which are persistent
 * 			during execution.
 */
void produce(void* args);
void setStaticFrameworkObjectIds();

}

#endif /* FACTORY_H_ */
