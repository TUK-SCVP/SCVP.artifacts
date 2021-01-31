#include <systemc.h>
#include <systemc-ams.h>

SCA_TDF_MODULE(X)
{
    public:
    sca_tdf::sca_out<double> out;

    SCA_CTOR(X) {
        set_timestep(sc_time(125,SC_US)); // 8kHz
    }

    void set_attributes() {
        out.set_rate(1);
    }

    void processing() {
        std::cout << name() << " " << sc_time_stamp() << std::endl;
        out.write(5.0);
    }
};

SCA_TDF_MODULE(A)
{
    public:
    sca_tdf::sca_in<double> in;
    sca_tdf::sca_out<double> out;

    SCA_CTOR(A) {
    }

    void set_attributes() {
        in.set_rate(1);
        out.set_rate(2);
    }

    void processing() {
        double tmp = in.read();
        std::cout << name() << " " << sc_time_stamp() << std::endl;
        for(unsigned long i = 0; i < 2; i++) {
            out.write(tmp, i);
            //std::cout << "A " << i << std::endl;
        }
    }
};

SCA_TDF_MODULE(B)
{
    public:
    sca_tdf::sca_in<double> in;
    sca_tdf::sca_out<double> out;

    SCA_CTOR(B) {
    }

    void set_attributes() {
        in.set_rate(1);
        out.set_rate(3);
    }

    void processing() {
        std::cout << name() << " " << sc_time_stamp() << std::endl;
        double tmp = in.read();
        for(unsigned long i = 0; i < 3; i++) {
            out.write(tmp, i);
            //std::cout << "B " << i << std::endl;
        }
    }
};

SCA_TDF_MODULE(C)
{
    public:
    sca_tdf::sca_in<double> in;
    sca_tdf::sca_out<double> out;

    SCA_CTOR(C) {
    }

    void set_attributes() {
        in.set_rate(2);
        out.set_rate(1);
    }

    void processing() {
        std::cout << name() << " " << sc_time_stamp() << std::endl;
        out.write(in.read(0)+in.read(1), 0); // or read(1)
    }
};

SCA_TDF_MODULE(Y)
{
    public:
    sca_tdf::sca_in<double> in;

    SCA_CTOR(Y) {
    }

    void set_attributes() {
    }

    void processing() {
        std::cout << name() << " " << sc_time_stamp() << std::endl;
        in.read();
    }

};

int sc_main(int argc, char* argv[])
{
    sc_core::sc_set_time_resolution(1.0, sc_core::SC_FS);

    X x("X");
    A a("A");
    B b("B");
    C c("C");
    Y y("Y");

    sca_tdf::sca_signal<double> xaSignal;
    sca_tdf::sca_signal<double> abSignal;
    sca_tdf::sca_signal<double> bcSignal;
    sca_tdf::sca_signal<double> cySignal;

    x.out(xaSignal);
    a.in(xaSignal);
    a.out(abSignal);
    b.in(abSignal);
    b.out(bcSignal);
    c.in(bcSignal);
    c.out(cySignal);
    y.in(cySignal);

    sca_util::sca_trace_file* tf = sca_util::sca_create_vcd_trace_file("trace.vcd");
    sca_util::sca_trace(tf, xaSignal, "xa");
    sca_util::sca_trace(tf, abSignal, "ab");
    sca_util::sca_trace(tf, bcSignal, "bc");
    sca_util::sca_trace(tf, cySignal, "cy");
    sc_core::sc_start(1, sc_core::SC_SEC);
    sca_util::sca_close_vcd_trace_file(tf);

    return 0;
}


