add_executable(feedback_loop
    feedback_loop.cpp
)

target_include_directories(feedback_loop
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(feedback_loop
    PRIVATE ${SYSTEMC_LIBRARY}
)
