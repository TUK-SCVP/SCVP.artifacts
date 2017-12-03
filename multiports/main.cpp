#include <iostream>
#include <systemc.h>

using namespace std;

SC_MODULE(module)
{
    sc_port<sc_fifo_out_if<int>,0,SC_ZERO_OR_MORE_BOUND> port;

    SC_CTOR(module)
    {
        SC_THREAD(process);
    }

    void process()
    {
        wait(SC_ZERO_TIME);

        port[0]->write(1);
        std::cout << "Write to port 0" << std::endl;
        wait(1,SC_NS);

        port[1]->write(1);
        std::cout << "Write to port 1" << std::endl;
        wait(1,SC_NS);

        port[2]->write(1);
        std::cout << "Write to port 2" << std::endl;
        wait(1,SC_NS);

        for(int i=0; i < port.size(); i++)
        {
           port[i]->write(2);
           std::cout << "Write to port " << i << std::endl;
           wait(1,SC_NS);
        }
    }

};

int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    module m("m");
    sc_fifo<int> f1, f2, f3;

    m.port.bind(f1);
    m.port.bind(f2);
    m.port.bind(f3);

    sc_start();
    return 0;
}
