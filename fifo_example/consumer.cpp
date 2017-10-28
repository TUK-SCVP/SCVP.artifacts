#include "consumer.h"

// This method is called every clock cycle:
void consumer::process()
{
    while(true)
    {
        unsigned int value = fifo_port->read();
        std::cout << "@" << sc_time_stamp()
                  << ": consumed" << value << std::endl;
        wait(); // Wait for next clock
    }
}
