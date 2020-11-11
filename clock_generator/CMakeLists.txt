add_executable(clock_generator
    main.cpp
)

target_include_directories(clock_generator
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(clock_generator
    PRIVATE ${SYSTEMC_LIBRARY}
)
