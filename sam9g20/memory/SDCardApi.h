#ifndef SAM9G20_MEMORY_SDCARDAPI_H_
#define SAM9G20_MEMORY_SDCARDAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SD_CARD_0 = 0,
    SD_CARD_1 = 1
} VolumeId;

/**
 * API to open and close the SD card filesystem. Written in C
 * to be easily usable by the bootloader as well.
 * @param volumeId
 * @return
 */
int open_filesystem(VolumeId volumeId);
int close_filesystem(VolumeId volumeId);
int select_sd_card(VolumeId volumeId);

#ifdef __cplusplus
}
#endif

#endif /* SAM9G20_MEMORY_SDCARDAPI_H_ */
