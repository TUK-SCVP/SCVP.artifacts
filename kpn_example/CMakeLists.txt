add_executable(kpn_example
    main.cpp
    kpn.cpp
    kpn.h
)

target_include_directories(kpn_example
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(kpn_example
    PRIVATE ${SYSTEMC_LIBRARY}
)
