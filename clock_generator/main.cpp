#include <iostream>
#include <systemc.h>

using namespace std;

SC_MODULE(clockGenerator)
{
    public:
    sc_out<bool> clk;
    sc_time period;
    bool value;

    SC_HAS_PROCESS(clockGenerator);
    clockGenerator(const sc_module_name &name, sc_time period) :
       sc_module(name),
       period(period),
       value(true)
    {
        SC_THREAD(generation);
    }
    private:
    void generation()
    {
        while(true)
        {
            value = !value;
            clk.write(value);
            wait(period/2);
        }
    }
};


int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{
    clockGenerator g("clock_1GHz", sc_time(1,SC_NS));
    sc_signal<bool> clk;

    // Bind Signals
    g.clk.bind(clk);

    // Setup Waveform Tracing:
    sc_trace_file *wf = sc_create_vcd_trace_file("traceFile");
    sc_trace(wf, clk, "clk");

    // Start Simulation
    sc_start(10, SC_NS);

    // Close Trace File:
    sc_close_vcd_trace_file(wf);

    return 0;
}
