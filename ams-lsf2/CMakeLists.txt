add_executable(ams-lsf2
    ltf_nd_filter.cpp
    ltf_pid.cpp
    pid_controller.cpp
    sc_main.cpp
)

target_include_directories(ams-lsf2
    PRIVATE ${SYSTEMC_AMS_INCLUDE}
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(ams-lsf2
    PRIVATE ${SYSTEMC_AMS_LIBRARY}
    PRIVATE ${SYSTEMC_LIBRARY}
)
