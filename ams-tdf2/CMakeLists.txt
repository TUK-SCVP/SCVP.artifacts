add_executable(ams-tdf2
    tdf2.cpp
)

target_include_directories(ams-tdf2
    PRIVATE ${SYSTEMC_AMS_INCLUDE}
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(ams-tdf2
    PRIVATE ${SYSTEMC_AMS_LIBRARY}
    PRIVATE ${SYSTEMC_LIBRARY}
)
