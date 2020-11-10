add_executable(tlm_simple_sockets
    main.cpp
    ../tlm_at_1/util.h
    ../tlm_memory_manager/memory_manager.cpp
    ../tlm_memory_manager/memory_manager.h
)

target_include_directories(tlm_simple_sockets
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(tlm_simple_sockets
    PRIVATE ${SYSTEMC_LIBRARY}
)
