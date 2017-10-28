#include "producer.h"

// This method is called every clock cycle:
void producer::process()
{
    while(true)
    {
        unsigned int value = counter++;

        fifo_port->write(value);
        std::cout << "@" << sc_time_stamp()
                  << ": Produced " << value << std::endl;

        if(counter == 20)
        {
            sc_stop();
        }
        wait(); // wait for next clock
    }
}
