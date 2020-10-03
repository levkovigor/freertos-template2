#ifndef SAM9G20_CORE_SCRUBBINGENGINE_H_
#define SAM9G20_CORE_SCRUBBINGENGINE_H_

#include <sam9g20/core/SoftwareImageHandler.h>

class ScrubbingEngine {
public:
    ScrubbingEngine(SoftwareImageHandler* owner);
private:
    SoftwareImageHandler* owner;
};



#endif /* SAM9G20_CORE_SCRUBBINGENGINE_H_ */
