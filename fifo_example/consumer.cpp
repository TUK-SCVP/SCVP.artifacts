#include "consumer.h"

// This method is called every clock cycle:
void consumer::process()
{
    while(true)
    {
        // Blocking read, i.e. an implicit wait is called
        unsigned int value = fifo_port->read();
        std::cout << "@" << sc_time_stamp()
                  << " CONSUMER: Consumed " << value << std::endl;
        wait(); // Wait for next clock
    }
}
