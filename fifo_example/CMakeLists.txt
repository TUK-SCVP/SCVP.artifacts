add_executable(fifo_example
    main.cpp
    consumer.cpp
    consumer.h
    producer.cpp
    producer.h
)

target_include_directories(fifo_example
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(fifo_example
    PRIVATE ${SYSTEMC_LIBRARY}
)
