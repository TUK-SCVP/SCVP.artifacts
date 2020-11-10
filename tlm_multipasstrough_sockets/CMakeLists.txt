add_executable(tlm_multipasstrough_sockets
    main.cpp
    initiator.h
    target.h
    ../tlm_at_1/util.h
    ../tlm_memory_manager/memory_manager.cpp
    ../tlm_memory_manager/memory_manager.h
)

target_include_directories(tlm_multipasstrough_sockets
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(tlm_multipasstrough_sockets
    PRIVATE ${SYSTEMC_LIBRARY}
)
