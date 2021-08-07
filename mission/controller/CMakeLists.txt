target_sources(${TARGET_NAME} PRIVATE
    PowerController.cpp
    ThermalController.cpp
)

add_subdirectory(acs)
