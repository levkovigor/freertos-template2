cmake_minimum_required(VERSION 3.13)

set(LIB_FREERTOS_NAME freertos)
add_library(${LIB_FREERTOS_NAME})

set(FREERTOS_PATH "${CMAKE_SOURCE_DIR}/freertos")

set(FREERTOS_PORT_SOURCES
    "${FREERTOS_PATH}/src/portable/GCC/ARM9_AT91SAM9G20/port.c"
    "${FREERTOS_PATH}/src/portable/MemMang/standardMemMang.c"
)

set(FREERTOS_CONFIG_AND_PORT_PATHS
    "${FREERTOS_PATH}/include/freertos/portable/GCC/ARM9_AT91SAM9G20"
    "${FREERTOS_PATH}/include/freertos/portable/GCC/MemMang"
    "${CMAKE_SOURCE_DIR}/sam9g20/boardconfig"
    "${CMAKE_SOURCE_DIR}"
)

if(CMAKE_VERBOSE)
message(STATUS "FreeRTOS port sources: ${FREERTOS_PORT_SOURCES}")
endif()

foreach(FREERTOS_PORT_SRC ${FREERTOS_PORT_SOURCES})
	if(IS_ABSOLUTE ${FREERTOS_PORT_SRC})
		set(${CURR_FREERTOS_PORT_SRC} ${FREERTOS_PORT_SRC})
	else()
		get_filename_component(CURR_FREERTOS_PORT_SRC
			${FREERTOS_PORT_SRC} REALPATH BASE_DIR ${CMAKE_SOURCE_DIR}
		)
	endif()
	
	list(APPEND FREERTOS_PORT_SOURCES_ABS ${CURR_FREERTOS_PORT_SRC})
endforeach()

target_sources(${LIB_FREERTOS_NAME}  PRIVATE
	src/croutine.c
	src/event_groups.c
	src/list.c
	src/queue.c
	src/stream_buffer.c
	src/tasks.c
	src/timers.c
	isisAdditions.c
	#"${FREERTOS_PORT_SOURCES_ABS}"
	${FREERTOS_PATH}/src/portable/GCC/ARM9_AT91SAM9G20/port.c
    ${FREERTOS_PATH}/src/portable/MemMang/standardMemMang.c
)

if(NOT FREERTOS_CONFIG_AND_PORT_PATHS)
	message(WARNING 
		"FREERTOS_CONFIG_AND_PORT_PATHS list of FreeRTOS includes was not " 
		"supplied, build might fail!"
	)
endif()

# Convert supplied FreeRTOS config and port includes to absolute paths.
foreach(FREERTOS_PATH ${FREERTOS_CONFIG_AND_PORT_PATHS})
	if(IS_ABSOLUTE ${FREERTOS_PATH})
		set(CURR_ABS_FREERTOS_PATH "${FREERTOS_PATH}")
	else()
		get_filename_component(CURR_ABS_FREERTOS_PATH 
			${FREERTOS_PATH} REALPATH BASE_DIR ${CMAKE_SOURCE_DIR})
	endif()
	
	if(CMAKE_VERBOSE)
		message(STATUS "FreeRTOS configuration path: ${CURR_ABS_FREERTOS_PATH}")
	endif()
	
	list(APPEND FREERTOS_PATHS_ABSOLUTE ${CURR_ABS_FREERTOS_PATH})
endforeach()

message(STATUS ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${LIB_FREERTOS_NAME} PRIVATE
    ${LIB_AT91_NAME}
)

# Required include paths to compile FreeRTOS
target_include_directories(${LIB_FREERTOS_NAME} INTERFACE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/freertos
    ${FREERTOS_PATHS_ABSOLUTE}
)

target_include_directories(${LIB_FREERTOS_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/freertos
    ${FREERTOS_PATHS_ABSOLUTE}
)
