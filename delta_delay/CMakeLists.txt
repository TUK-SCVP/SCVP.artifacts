add_executable(delta_delay
    delta_delay.cpp
)

target_include_directories(delta_delay
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(delta_delay
    PRIVATE ${SYSTEMC_LIBRARY}
)
