add_executable(ams-lsf
    lsf.cpp
)

target_include_directories(ams-lsf
    PRIVATE ${SYSTEMC_AMS_INCLUDE}
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(ams-lsf
    PRIVATE ${SYSTEMC_AMS_LIBRARY}
    PRIVATE ${SYSTEMC_LIBRARY}
)
