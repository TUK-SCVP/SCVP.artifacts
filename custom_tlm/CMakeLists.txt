add_executable(custom_tlm
    main.cpp
)

target_include_directories(custom_tlm
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(custom_tlm
    PRIVATE ${SYSTEMC_LIBRARY}
)
