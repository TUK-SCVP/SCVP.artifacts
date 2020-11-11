add_executable(callbacks
    main.cpp
)

target_include_directories(callbacks
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(callbacks
    PRIVATE ${SYSTEMC_LIBRARY}
)
