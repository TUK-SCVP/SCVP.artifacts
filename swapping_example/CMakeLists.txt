add_executable(swapping_example
    swapping_example.cpp
)

target_include_directories(swapping_example
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(swapping_example
    PRIVATE ${SYSTEMC_LIBRARY}
)
