#ifndef BSP_SAM9G20_MEMORY_HCCFILEGUARD_H_
#define BSP_SAM9G20_MEMORY_HCCFILEGUARD_H_

#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <hcc/api_fat.h>

/**
 * @brief   Helper class to facilitate RAII conformance when working with file handles.
 *          It will close a given file handle on destruction.
 * @details
 * Can be used by instantiating it on the stack. It also possible to not open a file and simply
 * bind the f_close call to the lifetime of the file helper.
 */
class HCCFileGuard {
public:
    /**
     * Constructor which will attempt to open given filename with given access parameter.
     * See p.70 of HCC API documentation
     * @param fileHandle
     * @param fileName
     * @param access
     * @param noOpen
     */
    HCCFileGuard(F_FILE** fileHandle, const char* fileName, const char* access);

    ReturnValue_t getOpenResult(int* errorCodeToSet);

    /**
     * This constructor can be used if the file was already opened to enable automatic
     * file close on destruction.
     * @param fileHandle
     */
    HCCFileGuard(F_FILE** fileHandle);
    ~HCCFileGuard();
private:
    F_FILE* fileHandle = nullptr;
    int errorCode = F_NO_ERROR;
};

#endif /* BSP_SAM9G20_MEMORY_HCCFILEGUARD_H_ */
