#ifndef PRODUCER_H
#define PRODUCER_H

#include <systemc.h>
#include <iostream>


SC_MODULE(producer)
{
  public:

    sc_in<bool> clk;
    // Thats the long way of writing it:
    sc_port< sc_fifo_out_if< unsigned int > > fifo_port;
    unsigned int counter;

    SC_CTOR(producer) : counter(1)
    {
        SC_METHOD(process);
        sensitive << clk.pos();
        dont_initialize();
    }

  private:
    void process();
};

#endif // PRODUCER_H
