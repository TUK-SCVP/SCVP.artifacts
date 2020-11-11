add_executable(tlm_lt_debug_transport
    main.cpp
)

target_include_directories(tlm_lt_debug_transport
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(tlm_lt_debug_transport
    PRIVATE ${SYSTEMC_LIBRARY}
)
