add_executable(tlm_lt_dmi
    main.cpp
)

target_include_directories(tlm_lt_dmi
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(tlm_lt_dmi
    PRIVATE ${SYSTEMC_LIBRARY}
)
