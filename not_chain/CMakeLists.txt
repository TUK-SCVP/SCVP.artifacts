add_executable(not_chain
    main.cpp
)

target_include_directories(not_chain
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(not_chain
    PRIVATE ${SYSTEMC_LIBRARY}
)
