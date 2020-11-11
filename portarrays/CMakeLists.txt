add_executable(portarrays
    main.cpp
)

target_include_directories(portarrays
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(portarrays
    PRIVATE ${SYSTEMC_LIBRARY}
)
