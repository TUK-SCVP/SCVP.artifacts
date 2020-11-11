add_executable(custom_signal
    main.cpp
    signal.h
)

target_include_directories(custom_signal
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(custom_signal
    PRIVATE ${SYSTEMC_LIBRARY}
)
