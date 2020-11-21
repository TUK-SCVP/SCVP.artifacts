#ifndef SIN_SRC_H
#define SIN_SRC_H

#include <systemc.h>
#include <systemc-ams.h>

SCA_TDF_MODULE(sin_src)
{
    public:
    sca_tdf::sca_out<double> out; // output port

    private:
    double amplitude;
    double frequency;
    sca_core::sca_time dstep;

    public:
    sin_src(sc_core::sc_module_name nm,
            double a= 1.0,
            double f = 1.0e3,
            sca_core::sca_time s = sca_core::sca_time(0.125, sc_core::SC_MS))
        : out("out"), amplitude(a), frequency(f), dstep(s)
    {}

    void set_attributes()
    {
        set_timestep(dstep);
    }

    void processing()
    {
        double t = get_time().to_seconds(); // actual time
        out.write( amplitude * std::sin( 2.0 * M_PI * frequency * t ) );
    }
};
#endif // SIN_SRC_H
