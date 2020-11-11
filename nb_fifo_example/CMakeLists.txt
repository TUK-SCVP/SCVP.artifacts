add_executable(nb_fifo_example
    main.cpp
    consumer.cpp
    consumer.h
    producer.cpp
    producer.h
)

target_include_directories(nb_fifo_example
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(nb_fifo_example
    PRIVATE ${SYSTEMC_LIBRARY}
)
