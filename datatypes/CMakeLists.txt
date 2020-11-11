add_executable(datatypes
    main.cpp
)

target_include_directories(datatypes
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(datatypes
    PRIVATE ${SYSTEMC_LIBRARY}
)
