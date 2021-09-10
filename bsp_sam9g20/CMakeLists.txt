if(NOT BOOTLOADER)
    target_sources(${TARGET_NAME} PRIVATE
        main.cpp
        InitMission.cpp
        ObjectFactory.cpp
    )
    
    add_subdirectory(boardconfig)
    add_subdirectory(comIF)
    add_subdirectory(memory)
    add_subdirectory(pus)
    add_subdirectory(utility)
    add_subdirectory(tmtcbridge)
    add_subdirectory(boardtest)
    add_subdirectory(core)
    add_subdirectory(fsfwconfig)
    add_subdirectory(devices)
endif()

add_subdirectory(common)