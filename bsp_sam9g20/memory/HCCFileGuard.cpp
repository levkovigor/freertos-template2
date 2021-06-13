#include "HCCFileGuard.h"
#include <OBSWConfig.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

HCCFileGuard::HCCFileGuard(F_FILE **fileHandle, const char *fileName, const char *access) {
    if(fileName != nullptr and access != nullptr and fileHandle != nullptr) {
        *fileHandle = f_open(fileName, access);
        if(*fileHandle == nullptr) {
            errorCode = f_getlasterror();
#if OBSW_VERBOSE_LEVEL >= 1
            sif::printWarning("Opening file %s failed with code %d\n\r", fileName, errorCode);
#endif
        }
    }
    if(fileHandle != nullptr) {
        this->fileHandle = *fileHandle;
    }
}

ReturnValue_t HCCFileGuard::getOpenResult(int* errorCodeToSet) {
    if(errorCode == F_NO_ERROR) {
        return HasReturnvaluesIF::RETURN_OK;
    }
    else {
        if(errorCodeToSet != nullptr) {
            *errorCodeToSet = errorCode;
        }
        return HasReturnvaluesIF::RETURN_FAILED;
    }
}

HCCFileGuard::~HCCFileGuard() {
    if(fileHandle != nullptr) {
        int result = f_close(fileHandle);
        if(result != F_NO_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "FileHelper: Closing failed, f_close error code: " << result <<
                    std::endl;
#else
            sif::printError("FileHelper: Closing failed, f_close error code: %d\n", result);
#endif
        }
    }
}

HCCFileGuard::HCCFileGuard(F_FILE **fileHandle) {
    if(fileHandle != nullptr) {
        this->fileHandle = *fileHandle;
    }
}
