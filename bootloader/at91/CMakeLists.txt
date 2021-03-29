if(BL_STAGE_TWO)
    add_subdirectory(second_stage)
else()
    add_subdirectory(first_stage)
endif()

add_subdirectory(common)
