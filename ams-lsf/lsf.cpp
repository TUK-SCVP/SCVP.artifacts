#include<systemc.h>
#include<systemc-ams.h>

SC_MODULE(lsf_model)
{ 
    sca_lsf::sca_in in;
    sca_lsf::sca_out out;

    lsf_model( sc_core::sc_module_name nm )
    {
        sca_lsf::sca_dot *dot = new sca_lsf::sca_dot("dot",0.1/(2*3.14));
        dot->x(in);
        dot->y(out);
    }
};

int sc_main(int argc, char* argv[])
{
    sca_lsf::sca_signal in, out;

    lsf_model model("lsf_model");

    sca_lsf::sca_source src("src", 0.0, 0.0, 10.0, 10.0, 0.0, SC_ZERO_TIME );
    // init_value
    // offset
    // amplitude
    // frequency
    // phase
    // delay

    src.set_timestep(100, sc_core::SC_US);

    src.y(in);
    model.in(in);
    model.out(out);

    sca_util::sca_trace_file* tf = sca_util::sca_create_tabular_trace_file("trace.csv");
    sca_util::sca_trace(tf, in, "input");
    sca_util::sca_trace(tf, out, "output");
    sc_core::sc_start(1.0, sc_core::SC_SEC);
    sca_util::sca_close_tabular_trace_file(tf);

    return 0;
}


