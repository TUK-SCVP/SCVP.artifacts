add_executable(sc_event_and_queue
    main.cpp
)

target_include_directories(sc_event_and_queue
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(sc_event_and_queue
    PRIVATE ${SYSTEMC_LIBRARY}
)
