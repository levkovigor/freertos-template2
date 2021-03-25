target_sources(${TARGET_NAME} PRIVATE
    GpioDeviceComIF.cpp
    I2cDeviceComIF.cpp
    PwmDeviceComIF.cpp
    RS232DeviceComIF.cpp
    RS232PollingTask.cpp
    RS485Controller.cpp
    RS485DeviceComIF.cpp
    SpiDeviceComIF.cpp
    UartPollingBase.cpp
    USBDeviceComIF.cpp
    USBPolling.cpp
)

add_subdirectory(cookies)