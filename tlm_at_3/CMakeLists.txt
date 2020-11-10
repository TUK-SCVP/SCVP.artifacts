add_executable(tlm_at_3
    main.cpp
    ../tlm_memory_manager/memory_manager.cpp
    ../tlm_memory_manager/memory_manager.h
    ../tlm_protocol_checker/tlm2_base_protocol_checker.h
    ../tlm_at_1/target.h
    ../tlm_at_1/initiator.h
    ../tlm_at_1/util.h
)

target_include_directories(tlm_at_3
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(tlm_at_3
    PRIVATE ${SYSTEMC_LIBRARY}
)
