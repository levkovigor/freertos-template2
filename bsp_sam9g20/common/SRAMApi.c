#include "SRAMApi.h"
#include <string.h>

void set_sram0_status_field(int32_t status) {
    memcpy((void*) (SRAM0_END - 4), (const void *) &status, sizeof(status));
}

int32_t get_sram0_status_field() {
    int32_t status = 0;
    memcpy((void*) &status, (const void*) (SRAM0_END - 4), sizeof(status));
    return status;
}

