#include<systemc.h>
#include<systemc-ams.h>

SC_MODULE(eln_circuit)
{ 
    sca_eln::sca_node n1;
    sca_eln::sca_node_ref gnd;
    sca_eln::sca_c c1;
    sca_eln::sca_l l1;

    //      +-----+ n1
    //    + |     | +
    //     [C]   [L]
    //    - |     | -
    //      +--+--+
    //         |
    //        _|_

    eln_circuit( sc_core::sc_module_name nm ) : 
        c1("c1", 500*1e-6, 500*1e-6), // = 1V
        l1("l1", 5)
    {
        c1.set_timestep(1, sc_core::SC_MS);
        c1.p(n1);
        c1.n(gnd);
        l1.p(n1);
        l1.n(gnd);
    }
};

int sc_main(int argc, char* argv[])
{
    eln_circuit cir("eln_circuit");
    sca_util::sca_trace_file* tf = sca_util::sca_create_tabular_trace_file("trace.dat");
    sca_util::sca_trace(tf, cir.c1, "i_through_c1");
    sc_core::sc_start(1, sc_core::SC_SEC);
    sca_util::sca_close_tabular_trace_file(tf);

    return 0;
}


