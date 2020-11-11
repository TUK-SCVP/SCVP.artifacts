add_executable(multiports
    main.cpp
)

target_include_directories(multiports
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(multiports
    PRIVATE ${SYSTEMC_LIBRARY}
)
