target_sources(${TARGET_NAME} PRIVATE
    USBD_UDP.c
    USBDDriver.c
    USBDCallbacks_Initialized.c
    USBDCallbacks_Reset.c
    USBDDriverCb_IfSettingChanged.c
)