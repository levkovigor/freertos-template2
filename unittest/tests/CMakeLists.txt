target_sources(${TARGET_NAME} PRIVATE
    DummyTest.cpp
    EtlMapWrapperTest.cpp
)

if(FSFW_ADD_UNITTESTS)
    target_sources(${TARGET_NAME} PRIVATE
        main.cpp
    )
endif()