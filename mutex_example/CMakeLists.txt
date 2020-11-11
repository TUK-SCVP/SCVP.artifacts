add_executable(mutex_example
    main.cpp
)

target_include_directories(mutex_example
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(mutex_example
    PRIVATE ${SYSTEMC_LIBRARY}
)
