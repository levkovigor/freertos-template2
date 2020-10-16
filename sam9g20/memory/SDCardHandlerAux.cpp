#include "SDCardHandler.h"


ReturnValue_t SDCardHandler::printRepository(const char *repository) {
    int result = change_directory(repository, true);
    if(result != F_NO_ERROR) {
        return result;
    }

    F_FIND findResult;
    int fileFound = f_findfirst("*.*", &findResult);
    if(fileFound != F_NO_ERROR) {
        return HasReturnvaluesIF::RETURN_OK;
    }

    if(findResult.filename[0] == '.') {
        // we are not in root, so the next search result is going
        // to be the parent folder, and the third result is going to be
        // the first file or directory.
        f_findnext(&findResult);
        fileFound = f_findnext(&findResult);
    }

    for(int idx = 0; idx < 255; idx++) {
        if(idx > 0) {
            fileFound = f_findnext(&findResult);
        }

        if(fileFound != F_NO_ERROR) {
            break;
        }

        // check whether file object is directory or file.
        if(change_directory(findResult.filename, false) == F_NO_ERROR) {
            change_directory("..", false);
            sif::info << "D: " << findResult.filename << std::endl;
        }
        else {
            sif::info << "F: " << findResult.filename << std::endl;
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::printHelper(uint8_t recursionDepth) {
    F_FIND findResult;
    int fileFound = f_findfirst("*.*", &findResult);
    if(fileFound != F_NO_ERROR) {
        return HasReturnvaluesIF::RETURN_OK;
    }

    if(findResult.filename[0] == '.') {
        // we are not in root, so the next search result is going
        // to be the parent folder, and the third result is going to be
        // the first file or directory.
        f_findnext(&findResult);
        fileFound = f_findnext(&findResult);
    }

    for(uint8_t idx = 0; idx < 255; idx ++) {
        if(idx > 0) {
            fileFound = f_findnext(&findResult);
        }

        if(fileFound != F_NO_ERROR) {
            return HasReturnvaluesIF::RETURN_OK;
        }


        if(change_directory(findResult.filename, false) == F_NO_ERROR) {
            for(uint8_t j = 0; j < recursionDepth; j++) {
                sif::info << "-";
            }
            sif::info << "D: " << findResult.filename << std::endl;
            printHelper(recursionDepth + 1);
            change_directory("..", false);
        }
        else {
            for(uint8_t j = 0; j < recursionDepth; j++) {
                sif::info << "-";
            }
            sif::info << "F: " << findResult.filename << std::endl;
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::printSdCard() {
    F_FIND findResult;
    int fileFound = 0;
    uint8_t recursionDepth = 0;
    f_chdir("/");
    // find directories first
    fileFound = f_findfirst("*.*", &findResult);
    if(fileFound != F_NO_ERROR) {
        // might be empty.
        sif::info << "SD Card empty." << std::endl;
        return HasReturnvaluesIF::RETURN_OK;
    }

    sif::info << "Printing SD Card: " << std::endl;
    sif::info << "F = File, D = Directory, - = Subdir Depth" << std::endl;

    for(int idx = 0; idx < 255; idx++) {
        if(idx > 0) {
            fileFound = f_findnext(&findResult);
        }

        if(fileFound != F_NO_ERROR) {
            break;
        }

        // check whether file object is directory or file.
        if(change_directory(findResult.filename, false) == F_NO_ERROR) {
            sif::info << "D: " << findResult.filename << std::endl;
            printHelper(recursionDepth + 1);
            change_directory("..", false);
        }
        else {
            // Normally files should have a three letter extension, but
            // we always check whether there is a file without extension
            sif::info << "F: " << findResult.filename << std::endl;
        }

    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::dumpSdCard() {
    // TODO: implement. This dumps the file structure of the SD card and will
    // be one of the most important functionalities for operators.
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::deleteFile(const char* repositoryPath,
        const char* filename, void* args) {
    int result = delete_file(repositoryPath, filename);
    if(result == F_NO_ERROR) {
        return HasReturnvaluesIF::RETURN_OK;
    }
    else {
        return result;
    }
}

ReturnValue_t SDCardHandler::createFile(const char* dirname,
        const char* filename, const uint8_t* data, size_t size,
        void* args) {
    int result = create_file(dirname, filename, data, size);
    if(result == -2) {
        return HasFileSystemIF::DIRECTORY_DOES_NOT_EXIST;
    }
    else if(result == -1) {
        return HasFileSystemIF::FILE_ALREADY_EXISTS;
    }
    else {
        //*bytesWritten = result;
        return HasReturnvaluesIF::RETURN_OK;
    }
}

ReturnValue_t SDCardHandler::createDirectory(const char* repositoryPath,
        const char* dirname){
    int result = create_directory(repositoryPath, dirname);
    if(result == F_ERR_DUPLICATED) {
        return HasFileSystemIF::DIRECTORY_ALREADY_EXISTS;
    }
    else if(result != F_NO_ERROR) {
        return result;
    }

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::deleteDirectory(const char* repositoryPath,
        const char* dirname){
    int result = delete_directory(repositoryPath, dirname);
    if(result == F_ERR_NOTEMPTY) {
        return HasFileSystemIF::DIRECTORY_NOT_EMPTY;
    }
    else if(result != F_NO_ERROR) {
        // should not happen (directory read only)
        return result;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::changeDirectory(const char* repositoryPath) {
    // change to root directory, all paths are going to be relative.
    int result = change_directory(repositoryPath, true);
    if(result == F_NO_ERROR) {
        return HasReturnvaluesIF::RETURN_OK;
    }
    else {
        if(result == F_ERR_INVALIDDIR) {
            return HasFileSystemIF::DIRECTORY_DOES_NOT_EXIST;
        }
        return result;
    }
}

