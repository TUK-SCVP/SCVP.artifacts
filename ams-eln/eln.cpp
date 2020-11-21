#include<systemc.h>
#include<systemc-ams.h>

SC_MODULE(eln_circuit)
{ 
    sca_eln::sca_node n1;
    sca_eln::sca_node n2;
    sca_eln::sca_node_ref gnd;
    sca_eln::sca_vsource vin;
    sca_eln::sca_r r1;
    sca_eln::sca_c c1;

    /*   n1 +   - n2
     *    +--[R]--+
     *   +|       |+
     *   Vin     [C]
     *   -|       |-
     *    |       |
     *   -+-     -+- GND
     */

    eln_circuit( sc_core::sc_module_name nm )
        : vin("vin",              // Name
                0.0,              // Init
                10000.0,          // Offset
                0,                // Amplitude
                0,                // Frequency
                0,                // Phase
                sc_time(1, SC_MS) // Delay
             ), 
        r1("r1", 10000),
        c1("c1", 0.0001)
    {
        vin.set_timestep(0.1, sc_core::SC_SEC);
        vin.p(n1);
        vin.n(gnd);
        r1.p(n1);
        r1.n(n2);
        c1.p(n2);
        c1.n(gnd);
    }
};

int sc_main(int argc, char* argv[])
{
    eln_circuit cir("eln_circuit");
    sca_util::sca_trace_file* tf = sca_util::sca_create_tabular_trace_file("trace.dat");
    sca_util::sca_trace(tf, cir.r1, "i_through_r1");
    sc_core::sc_start(3.5, sc_core::SC_SEC);
    sca_util::sca_close_tabular_trace_file(tf);

    return 0;
}


