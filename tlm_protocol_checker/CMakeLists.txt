add_executable(tlm_protocol_checker
    main.cpp
    tlm2_base_protocol_checker.h
)

target_include_directories(tlm_protocol_checker
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(tlm_protocol_checker
    PRIVATE ${SYSTEMC_LIBRARY}
)
