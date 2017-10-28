#ifndef CONSUMER_H
#define CONSUMER_H

#include <systemc.h>
#include <iostream>


SC_MODULE(consumer)
{
  public:

    sc_in<bool> clk;
    // Thats the short way of writing it:
    sc_fifo_in<unsigned int> fifo_port;

    SC_CTOR(consumer)
    {
        SC_METHOD(process);
        sensitive << clk.pos();
        dont_initialize();
    }

  private:
    void process();
};

#endif // CONSUMER_H
