add_executable(event_finder
    main.cpp
    ../custom_signal/signal.h
)

target_include_directories(event_finder
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(event_finder
    PRIVATE ${SYSTEMC_LIBRARY}
)
