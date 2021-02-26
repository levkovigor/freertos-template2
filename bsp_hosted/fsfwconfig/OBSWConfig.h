#ifndef CONFIG_TMTC_TMTCSIZE_H_
#define CONFIG_TMTC_TMTCSIZE_H_

#include <cstdint>
#include <cstddef>

#define OBSW_TRACK_FACTORY_ALLOCATION_SIZE      0
#define OBSW_ADD_TEST_CODE                      1
#define OBSW_PRINT_MISSED_DEADLINES             1
#define OBSW_VERBOSE_LEVEL                      1

namespace config {
static constexpr uint32_t MAX_STORED_TELECOMMANDS = 2000;
}

#endif /* CONFIG_TMTC_TMTCSIZE_H_ */
