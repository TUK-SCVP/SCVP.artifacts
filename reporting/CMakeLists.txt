add_executable(reporting
    main.cpp
)

target_include_directories(reporting
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(reporting
    PRIVATE ${SYSTEMC_LIBRARY}
)
