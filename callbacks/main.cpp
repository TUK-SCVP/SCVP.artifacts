#include <iostream>
#include <systemc.h>

using namespace std;

SC_MODULE(module)
{
    public:
    sc_in<bool> clk;
    sc_trace_file *tf;

    SC_CTOR(module)
    {
    }

    void process()
    {
        wait(5);
        sc_stop();
    }

    void before_end_of_elaboration()
    {
        cout << "before_end_of_elaboration" << endl;
        SC_THREAD(process);
        sensitive << clk.pos();
    }

    void end_of_elaboration()
    {
        cout << "end_of_elaboration" << endl;
    }

    void start_of_simulation()
    {
        cout << "start_of_simulation" << endl;
        tf = sc_create_vcd_trace_file("trace");
    }

    void end_of_simulation()
    {
        cout << "end_of_simulation" << endl;
        sc_close_vcd_trace_file(tf);
    }

};


int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    module m("m");
    sc_clock clk("clk",sc_time(1,SC_NS));

    m.clk(clk);

    sc_start();
    return 0;
}
