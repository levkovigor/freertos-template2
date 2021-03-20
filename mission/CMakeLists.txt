target_sources(${TARGET_NAME} PRIVATE
    SatelliteSystem.cpp
)

add_subdirectory(assembly)
add_subdirectory(controller)
add_subdirectory(devices)
add_subdirectory(fdir)
add_subdirectory(pus)
add_subdirectory(utility)

