add_executable(tlm_at_1
    main.cpp
    initiator.h
    target.h
    util.h
    ../tlm_memory_manager/memory_manager.cpp
    ../tlm_memory_manager/memory_manager.h
    ../tlm_protocol_checker/tlm2_base_protocol_checker.h
)

target_include_directories(tlm_at_1
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(tlm_at_1
    PRIVATE ${SYSTEMC_LIBRARY}
)
