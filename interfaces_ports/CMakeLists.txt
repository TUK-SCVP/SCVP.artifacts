add_executable(interfaces_ports
    main.cpp
)

target_include_directories(interfaces_ports
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(interfaces_ports
    PRIVATE ${SYSTEMC_LIBRARY}
)
