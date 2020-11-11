add_executable(dynamic_processes
    main.cpp
)

target_include_directories(dynamic_processes
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(dynamic_processes
    PRIVATE ${SYSTEMC_LIBRARY}
)
