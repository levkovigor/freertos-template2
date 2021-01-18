#ifndef SAM9G20_OBJECTFACTORY_H_
#define SAM9G20_OBJECTFACTORY_H_

namespace Factory {

/**
 * @brief   Creates all SystemObject elements which are persistent
 *          during execution.
 */
void produce();
void setStaticFrameworkObjectIds();

}

#endif /* SAM9G20_OBJECTFACTORY_H_ */
