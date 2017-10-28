#include <systemc>
#include <iostream>
#include "utils.h"
#include "producer.h"
#include "consumer.h"

int sc_main(int argc, char* argv[])
{
    // Setup Clocks
    sc_clock clk1("clk1", 1, SC_NS, 0.5, 0, SC_NS, true);
    sc_clock clk2("clk2", 2, SC_NS, 0.5, 0, SC_NS, true);

    // Setup Producer
    producer p("producer");

    // Setup Consumer
    consumer c("consumer");

    // Setup communication channel:
    sc_fifo<unsigned int> channel(4);

    // Bind Signals:
    p.clk(clk1);
    p.fifo_port.bind(channel);

    c.clk(clk2);
    c.fifo_port.bind(channel);

    // Setup Waveform Tracing:
    sc_trace_file *wf = sc_create_vcd_trace_file("tff");
    wf->set_time_unit(1, SC_PS);
    sc_trace(wf, clk1, "clk1");
    sc_trace(wf, clk2, "clk2");

    // https://workspace.accellera.org/Discussion_Forums/community_forum/archive/msg?list_name=community_forum&monthdir=200411&msg=msg00013.html
    channel.trace(wf);

    sc_start();

    channel.dump();

    sc_close_vcd_trace_file(wf);
    return 0;
}
