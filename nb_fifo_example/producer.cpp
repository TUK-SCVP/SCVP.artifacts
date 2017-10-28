#include "producer.h"

// This method is called every clock cycle:
void producer::process()
{
    // Since this is a SC_METHOD, we cannot call fifo_port->write() because
    // this would call an implicit wait statement when the fifo is full.
    // If we want to have this functionality we would need a SC_THREAD!

    unsigned int value = counter++;
    if(fifo_port->nb_write(value))
    {
        std::cout << "@" << sc_time_stamp()
                  << " PRODUCER: Produced " << value << std::endl;
    }
    else
    {
        std::cout << "@" << sc_time_stamp()
                  << " PRODUCER: fifo is full" << std::endl;
        counter--;
    }

    if(counter == 20)
    {
        sc_stop();
    }
}
