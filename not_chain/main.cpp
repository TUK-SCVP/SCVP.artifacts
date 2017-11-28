#include "systemc.h"

SC_MODULE(NOT)
{
    public:
    sc_in<bool> in;
    sc_out<bool> out;

    SC_CTOR(NOT) : in("in"), out("out")
    {
        SC_METHOD(process);
    }

    void process()
    {
       out.write(!in.read());
    }
};

SC_MODULE(not_chain)
{
    sc_in<bool> A;
    sc_out<bool> Z;

    NOT not1, not2, not3;

    sc_signal<bool> h1,h2;

    SC_CTOR(not_chain):
        A("A"), Z("Z"),
        not1("not1"), not2("not2"), not3("not3"),
        h1("h1"), h2("h2")
    {
        //        h1    h2
        // A--NOT1--NOT2--NOT3--Z
        not1.in.bind(A);
        not1.out.bind(h1);

        not2.in(h1);
        not2.out(h2);

        not3.in(h2);
        not3.out(Z);
    }
};

int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{
    sc_signal<bool> foo, bar;

    not_chain c("not_chain");

    foo.write(0);
    c.A.bind(foo);
    c.Z.bind(bar);

    sc_start();

    std::cout << bar.read() << std::endl;
    return 0;
}
