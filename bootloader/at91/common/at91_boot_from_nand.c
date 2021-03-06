#include "at91_boot_from_nand.h"
#include <bootloaderConfig.h>

#include <bsp_sam9g20/common/SRAMApi.h>

#include <at91/boards/at91sam9g20-ek/board.h>
#include <at91/boards/at91sam9g20-ek/board_memories.h>
#include <memories/nandflash/SkipBlockNandFlash.h>
#include <at91/utility/trace.h>

#include <hal/Timing/RTT.h>

#include <inttypes.h>
#include <string.h>

/// Nandflash memory size.
static unsigned int memSize;
/// Size of one block in the nandflash, in bytes.
static unsigned int blockSize;
/// Number of blocks in nandflash.
static unsigned short numBlocks;
/// Size of one page in the nandflash, in bytes.
static unsigned short pageSize;
/// Number of page per block
static unsigned short numPagesPerBlock;
// Nandflash bus width
static unsigned char nfBusWidth = 16;

/// Pins used to access to nandflash.
static const Pin pPinsNf[] = {PINS_NANDFLASH};
/// Nandflash device structure.
static struct SkipBlockNandFlash skipBlockNf;
/// Address for transferring command bytes to the nandflash.
static unsigned int cmdBytesAddr = BOARD_NF_COMMAND_ADDR;
/// Address for transferring address bytes to the nandflash.
static unsigned int addrBytesAddr = BOARD_NF_ADDRESS_ADDR;
/// Address for transferring data bytes to the nandflash.
static unsigned int dataBytesAddr = BOARD_NF_DATA_ADDR;
/// Nandflash chip enable pin.
static const Pin nfCePin = BOARD_NF_CE_PIN;
/// Nandflash ready/busy pin.
static const Pin nfRbPin = BOARD_NF_RB_PIN;

// Transfer return codes
#define BOOT_NAND_SUCCESS            0 /// All requested transfer are successfull
#define BOOT_NAND_ERROR_NO_DEVICE    1 /// No nand devices has been detected
#define BOOT_NAND_ERROR_GP           2

int copy_nandflash_image_to_sdram(const uint32_t source_offset, const size_t source_size,
        const size_t target_offset, bool configureNand) {
    /* Disable most traces because there is a lot of spam from the NAND drivers */
#if BOOTLOADER_VERBOSE_LEVEL <= 1
    setTrace(TRACE_LEVEL_WARNING);
#endif

    if(configureNand) {
        // NandInit();
    }
    // set_sram0_status_field(16);
#if BOOTLOADER_VERBOSE_LEVEL >= 1
    printf("-I- Start copying %d bytes from NAND address 0x%08x..\n\r",
            (int) source_size, (unsigned int) source_offset);
#endif

    BOOT_NAND_CopyBin(source_offset, source_size, target_offset);

    /* Enable traces */
#if BOOTLOADER_VERBOSE_LEVEL <= 1
    setTrace(TRACE_LEVEL_DEBUG);
#endif

    return 0;
}

//------------------------------------------------------------------------------
/// Initialize NAND flash driver.
//------------------------------------------------------------------------------
void NandInit()
{
    //-------------------------------------------------------------------------
    TRACE_INFO("Init NAND Flash\n\r");

    // Configure SMC for Nandflash accesses (done each time because of old ROM codes)
    BOARD_ConfigureNandFlash(nfBusWidth);
    PIO_Configure(pPinsNf, PIO_LISTSIZE(pPinsNf));

    //memset(&skipBlockNf, 0, sizeof(skipBlockNf));

    if (SkipBlockNandFlash_Initialize(&skipBlockNf,0,cmdBytesAddr, addrBytesAddr,
            dataBytesAddr, nfCePin, nfRbPin)) {

        TRACE_ERROR("Device Unknown\n\r");
    }

    // Check the data bus width of the NandFlash
    nfBusWidth = NandFlashModel_GetDataBusWidth((struct NandFlashModel *)&skipBlockNf);

    // Reconfigure bus width
    BOARD_ConfigureNandFlash(nfBusWidth);

    TRACE_INFO("Nandflash driver initialized\n\r");

    // Get device parameters
    memSize = NandFlashModel_GetDeviceSizeInBytes(&skipBlockNf.ecc.raw.model);
    blockSize = NandFlashModel_GetBlockSizeInBytes(&skipBlockNf.ecc.raw.model);
    numBlocks = NandFlashModel_GetDeviceSizeInBlocks(&skipBlockNf.ecc.raw.model);
    pageSize = NandFlashModel_GetPageDataSize(&skipBlockNf.ecc.raw.model);
    numPagesPerBlock = NandFlashModel_GetBlockSizeInPages(&skipBlockNf.ecc.raw.model);

    TRACE_INFO("Size of the whole device in bytes : 0x%x \n\r",memSize);
    TRACE_INFO("Size in bytes of one single block of a device : 0x%x \n\r",blockSize);
    TRACE_INFO("Number of blocks in the entire device : 0x%x \n\r",numBlocks);
    TRACE_INFO("Size of the data area of a page in bytes : 0x%x \n\r",pageSize);
    TRACE_INFO("Number of pages in the entire device : 0x%x \n\r",numPagesPerBlock);
    TRACE_INFO("Bus width : %d \n\r",nfBusWidth);
}

//------------------------------------------------------------------------------
/// Initialize NAND devices and transfer one or sevral modules from Nand to the
/// target memory (SRAM/SDRAM).
/// \param binary_offset     Offset of start
/// \param binary_size       Size of binary
//------------------------------------------------------------------------------
int BOOT_NAND_CopyBin(const uint32_t binary_offset, size_t binary_size, size_t target_offset)
{
    unsigned short block = 0;
    unsigned short page = 0;
    unsigned short offset = 0;

    unsigned char *ptr = NULL;
    unsigned bytes_read = 0;

    // Errors returned by SkipNandFlash functions
    unsigned char error = 0;
    /*...........................................................................*/

    // Initialize Nand
    NandInit();
    // Transfert data from Nand to External RAM
    //-------------------------------------------------------------------------

#if !defined(OP_BOOTSTRAP_on)
    // Check word alignment
    if (binary_offset % 4) {

        TRACE_ERROR("Offset not word aligned\n\r");
        return BOOT_NAND_ERROR_GP;
    }
#endif

#if !defined(OP_BOOTSTRAP_on)
    // Retrieve page and block addresses
    if (NandFlashModel_TranslateAccess(&(skipBlockNf.ecc.raw.model),
            binary_offset,
            binary_size,
            &block,
            &page,
            &offset)) {
        TRACE_ERROR("TranslateAccess Error\n\r");
        return BOOT_NAND_ERROR_GP;
    }
#else
    NandFlashModel_TranslateAccess(&(skipBlockNf.ecc.raw.model),
            binary_offset,
            binary_size,
            &block,
            &page,
            &offset);
#endif

#if !defined(OP_BOOTSTRAP_on)
    if (page || offset) {

        TRACE_ERROR("Address is not a block start\n\r");
        return BOOT_NAND_ERROR_GP;
    }
#endif

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    printf("-I- Access translation: Starting at block %d and page %d..\n\r", block, page);
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

    // Initialize to SDRAM start address
    ptr = (unsigned char*) (SDRAM_DESTINATION + target_offset);
    bytes_read = binary_size;
    while(block < numBlocks)
    {
        for(page = 0; page < numPagesPerBlock; page++) {

            do {
                error = SkipBlockNandFlash_ReadPage(&skipBlockNf, block,
                        page, ptr, 0);
#if BOOTLOADER_VERBOSE_LEVEL >= 2
                if((block == 1) && (page == 0)) {
                    unsigned int armVector = 0;
                    memcpy(&armVector, ptr, 4);
                    TRACE_WARNING("1: %08x\n\r", armVector);
                    memcpy(&armVector, ptr + 4, 4);
                    TRACE_WARNING("2: %08x\n\r", armVector);
                    memcpy(&armVector, ptr + 8, 4);
                    TRACE_WARNING("3: %08x\n\r", armVector);
                    memcpy(&armVector, ptr + 12, 4);
                    TRACE_WARNING("4: %08x\n\r", armVector);
                    memcpy(&armVector, ptr + 16, 4);
                    TRACE_WARNING("5: %08x\n\r", armVector);
                    memcpy(&armVector, ptr + 20, 4);
                    TRACE_WARNING("6: %08x\n\r", armVector);
                    memcpy(&armVector, ptr + 24, 4);
                    TRACE_WARNING("7: %08x\n\r", armVector);
                }
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 2 */
                //        		TRACE_WARNING("SkipBlockNandFlash_ReadBlock: Reading block %d "
                //       				"page %d.\n\r", block, page);
                if (error == NandCommon_ERROR_BADBLOCK) {
                    block++;
                }
                else {
                    break;
                }
            }
            while(block < numBlocks && page == 0);

#if !defined(OP_BOOTSTRAP_on)
            if (error) {
                TRACE_ERROR("SkipBlockNandFlash_ReadBlock: Cannot read page "
                        "%d of block %d.\n\r", page, block);
                return BOOT_NAND_ERROR_GP;
            }
#endif
            ptr += pageSize;

            if(bytes_read <= pageSize) {
                bytes_read = 0;
                break;
            }
            else {
                bytes_read -= pageSize;
            }
        }

        if(bytes_read == 0) {
            break;
        }
        else {
            block++;
        }
    }
    return BOOT_NAND_SUCCESS;
}

void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType) {
    typedef void (*fctType) (volatile unsigned int, volatile unsigned int);
    void (*pFct) (volatile unsigned int r0_val, volatile unsigned int r1_val);

    pFct = (fctType) jumpAddr;
    pFct(0/*dummy value in r0*/, matchType/*matchType in r1*/);

    while(1);//never reach
}


void idle_loop() {
    uint32_t last_time = RTT_GetTime();
    for(;;) {
        uint32_t curr_time = RTT_GetTime();
        if(curr_time - last_time >= 1) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
            TRACE_INFO("Bootloader idle..\n\r");
#endif
            last_time = curr_time;
        }
    }
}

