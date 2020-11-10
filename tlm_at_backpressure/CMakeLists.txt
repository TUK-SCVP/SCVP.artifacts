add_executable(tlm_at_backpressure
    main.cpp
    initiator.h
    target.h
    ../tlm_memory_manager/memory_manager.cpp
    ../tlm_memory_manager/memory_manager.h
    ../tlm_protocol_checker/tlm2_base_protocol_checker.h
    ../tlm_at_1/util.h
)

target_include_directories(tlm_at_backpressure
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(tlm_at_backpressure
    PRIVATE ${SYSTEMC_LIBRARY}
)
