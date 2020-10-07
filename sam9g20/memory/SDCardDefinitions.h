#ifndef SAM9G20_MEMORY_SDCARDDEFINITIONS_H_
#define SAM9G20_MEMORY_SDCARDDEFINITIONS_H_

#include <etl/string.h>
#include <config/OBSWConfig.h>

using RepositoryPath = etl::string<MAX_REPOSITORY_PATH_LENGTH>;
using FileName = etl::string<MAX_FILENAME_LENGTH>;

static constexpr size_t MAX_READ_LENGTH = 1024;

#endif /* SAM9G20_MEMORY_SDCARDDEFINITIONS_H_ */
