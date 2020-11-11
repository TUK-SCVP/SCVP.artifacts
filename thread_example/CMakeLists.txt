add_executable(thread_example
    thread_example.cpp
)

target_include_directories(thread_example
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(thread_example
    PRIVATE ${SYSTEMC_LIBRARY}
)
