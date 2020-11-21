add_executable(ams-eln2
    eln.cpp
)

target_include_directories(ams-eln2
    PRIVATE ${SYSTEMC_AMS_INCLUDE}
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(ams-eln2
    PRIVATE ${SYSTEMC_AMS_LIBRARY}
    PRIVATE ${SYSTEMC_LIBRARY}
)
