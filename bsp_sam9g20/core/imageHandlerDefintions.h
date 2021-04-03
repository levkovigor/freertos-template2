#ifndef SAM9G20_CORE_IMAGEHANDLERDEFINTIONS_H_
#define SAM9G20_CORE_IMAGEHANDLERDEFINTIONS_H_

#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <returnvalues/classIds.h>
#include <events/subsystemIdRanges.h>
#include <fsfw/events/Event.h>

#ifdef ISIS_OBC_G20
#include <hal/Storage/NORflash.h>
#else
#include <at91/memories/nandflash/NandCommon.h>
#endif

#include <array>

namespace image {

static constexpr uint8_t INTERFACE_ID = CLASS_ID::SW_IMAGE_HANDLER;

static constexpr ReturnValue_t OPERATION_FINISHED = MAKE_RETURN_CODE(0);
static constexpr ReturnValue_t TASK_PERIOD_OVER_SOON = MAKE_RETURN_CODE(1);
static constexpr ReturnValue_t BUSY = MAKE_RETURN_CODE(2);
static constexpr ReturnValue_t FRAM_ISSUE = MAKE_RETURN_CODE(3);

static constexpr uint8_t subsystemId = SUBSYSTEM_ID::IMAGE_HANDLER;

static constexpr Event FRAM_ISSUE_EVENT = event::makeEvent(subsystemId, 0, severity::MEDIUM);

#ifdef AT91SAM9G20_EK
using ImageBuffer = std::array<uint8_t, NandCommon_MAXPAGEDATASIZE>;
#else
using ImageBuffer = std::array<uint8_t, NORFLASH_SMALL_SECTOR_SIZE>;
#endif

/* Image slots available */
enum ImageSlot: uint8_t {
    NONE,
    BOOTLOADER_0,   //!< Primary bootloader.
    BOOTLOADER_1,   //!< Second-stage bootloader (optional for AT91 board)
    FLASH,       //!< NOR-Flash image slot
    SDC_SLOT_0,     //!< Primary Image SD Card (for each SD Card)
    SDC_SLOT_1,     //!< Secondary and update image (for each SD Card)
};

}


#endif /* SAM9G20_CORE_IMAGEHANDLERDEFINTIONS_H_ */
