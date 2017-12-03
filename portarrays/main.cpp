#include <iostream>
#include <systemc.h>

using namespace std;

template <int N=1>
SC_MODULE(module)
{
    // Instead of
    //sc_port<sc_fifo_out_if<int> > port1;
    //sc_port<sc_fifo_out_if<int> > port2;
    //sc_port<sc_fifo_out_if<int> > port3;

    sc_port<sc_fifo_out_if<int> > port[N];

    SC_CTOR(module)
    {
        SC_THREAD(process);
    }

    void process()
    {
        wait(SC_ZERO_TIME);

        for(int i=0; i < N; i++)
        {
           port[i]->write(2);
           std::cout << this->name()
                     << ": Write to port " << i << std::endl;
           wait(1, SC_NS);
        }

        std::cout << flush;
    }

};

int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    module<3> m("m");
    sc_fifo<int> f1, f2, f3, f4;

    m.port[0].bind(f1);
    m.port[1].bind(f2);
    m.port[2].bind(f3);

    module<1> n("n");
    module<1> o("o");

    n.port[0].bind(f4);
    o.port[0].bind(f4);

    sc_start();
    return 0;
}
