#include <iostream>
#include <systemc.h>
#include "signal.h"

using namespace std;

SC_MODULE(PRODUCER)
{
    sc_port< Signal<int> > master;

    SC_CTOR(PRODUCER)
    {
        SC_THREAD(process);
    }

    void process()
    {
        master->write(10);
        wait(10,SC_NS);
        master->write(20);
        wait(20,SC_NS);
        sc_stop();
    }
};

SC_MODULE(CONSUMER)
{
    sc_port< Signal<int> > slave;

    SC_CTOR(CONSUMER)
    {
        SC_METHOD(process);
        sensitive << slave;
        dont_initialize();
    }

    void process()
    {
        int v = slave->read();
        std::cout << v << std::endl;
    }
};

int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    PRODUCER pro1("pro1");
    CONSUMER con1("con1");
    Signal<int> channel;

    pro1.master.bind(channel);
    con1.slave.bind(channel);

    sc_start(sc_time(100,SC_NS));

    return 0;
}
