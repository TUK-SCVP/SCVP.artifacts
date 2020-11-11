add_executable(kpn_artificial_deadlock
    main.cpp
    kpn.cpp
    kpn.h
    utils.h
)

target_include_directories(kpn_artificial_deadlock
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(kpn_artificial_deadlock
    PRIVATE ${SYSTEMC_LIBRARY}
)
