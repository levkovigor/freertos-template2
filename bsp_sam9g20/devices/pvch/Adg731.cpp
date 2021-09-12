#include "Adg731.h"
#include "devices/logicalAddresses.h"

#include "bsp_sam9g20/comIF/SpiDeviceComIF.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/globalfunctions/bitutility.h"

Adg731::Adg731(Pca9554& spiMux): spiMux(spiMux),
        spiCookie(addresses::SPI_DLR_PVCH, 16, SlaveType::PVCH, SPImode::mode1_spi)  {
    spiComIF = ObjectManager::instance()->get<SpiDeviceComIF>(objects::SPI_DEVICE_COM_IF);
    if(spiComIF == nullptr) {
        sif::printError("Ads731: SPI communication interface invalid\n");
    }
}

ReturnValue_t Adg731::initialize() {
    return set(AdgChannels::ALL_OFF_CMD);
}

ReturnValue_t Adg731::set(AdgChannels channel) {
    cmd = 0;
    switch(channel) {
    case(AdgChannels::CH_1): {
        break;
    }
    case(AdgChannels::CH_2): {
        bitutil::bitSet(&cmd, 0);
        break;
    }
    case(AdgChannels::CH_3): {
        bitutil::bitSet(&cmd, 1);
        break;
    }
    case(AdgChannels::CH_4): {
        bitutil::bitSet(&cmd, 0);
        bitutil::bitSet(&cmd, 1);
        break;
    }
    case(AdgChannels::CH_5): {
        bitutil::bitSet(&cmd, 2);
        break;
    }
    case(AdgChannels::ALL_OFF_CMD): {
        bitutil::bitSet(&cmd, 7);
        break;
    }
    }

    ReturnValue_t result = spiMux.clearMuxSync();
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    result = spiComIF->sendMessage(&spiCookie, &cmd, 1);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::printWarning("Adg731: SPI transfer failed\n");
        return result;
    }
    // Block until operation is finished
    result = spiSemaph->acquire(SemaphoreIF::TimeoutType::WAITING, 50);
    if(result == HasReturnvaluesIF::RETURN_OK) {
        spiSemaph->release();
    }
    else {
        sif::printWarning("Adg731: SPI transfer timed out\n");
    }
    return spiMux.setMuxSync();
}
