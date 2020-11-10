add_executable(tlm_payload_extensions
    main.cpp
    ../tlm_memory_manager/memory_manager.cpp
    ../tlm_memory_manager/memory_manager.h
    ../tlm_multipasstrough_sockets/initiator.h
    ../tlm_multipasstrough_sockets/target.h
)

target_include_directories(tlm_payload_extensions
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(tlm_payload_extensions
    PRIVATE ${SYSTEMC_LIBRARY}
)
