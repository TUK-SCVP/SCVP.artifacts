#include "consumer.h"

// This method is called every clock cycle:
void consumer::process()
{
    // Since this is a SC_METHOD, we cannot call fifo_port->read() because
    // this would call an implicit wait statement when the fifo is empty.
    // If we want to have this functionality we would need a SC_THREAD!
    unsigned int value;

    if(fifo_port->nb_read(value) == true)
    {
        std::cout << "@" << sc_time_stamp()
                  << " CONSUMER: Consumed " << value << std::endl;
    }
    else
    {
        std::cout << "@" << sc_time_stamp()
                  << " CONSUMER: nothing to consume" << std::endl;
    }
}
