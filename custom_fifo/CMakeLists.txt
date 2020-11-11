add_executable(custom_fifo
    main.cpp
)

target_include_directories(custom_fifo
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(custom_fifo
    PRIVATE ${SYSTEMC_LIBRARY}
)
