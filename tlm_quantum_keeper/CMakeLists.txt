add_executable(tlm_quantum_keeper
    main.cpp
)

target_include_directories(tlm_quantum_keeper
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(tlm_quantum_keeper
    PRIVATE ${SYSTEMC_LIBRARY}
)
