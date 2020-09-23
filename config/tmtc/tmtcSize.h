#ifndef CONFIG_TMTC_TMTCSIZE_H_
#define CONFIG_TMTC_TMTCSIZE_H_

#include <cstdint>
#include <cstddef>

namespace tmtcsize {
static constexpr size_t MAX_SERIAL_FRAME_SIZE = 1500;
static constexpr size_t MAX_USB_FRAME_SIZE = 1500;
static constexpr uint32_t MAX_STORED_TELECOMMANDS = 2000;
static constexpr uint32_t MAX_TM_PACKET = 50;
}

#endif /* CONFIG_TMTC_TMTCSIZE_H_ */
