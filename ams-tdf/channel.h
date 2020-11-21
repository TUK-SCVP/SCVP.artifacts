#ifndef CHANNEL_H
#define CHANNEL_H
#include <systemc.h>
#include <systemc-ams.h>
#include <cstdlib>
#include <cmath>

double gauss_rand(double variance)
{
    double rnd1, rnd2, Q, Q1, Q2;
    do {
         rnd1 = ((double)std::rand()) / ((double)RAND_MAX) ;
         rnd2 = ((double)std::rand()) / ((double)RAND_MAX) ;
         Q1 = 2.0 * rnd1 - 1.0 ;
         Q2 = 2.0 * rnd2 - 1.0 ;
         Q = Q1 * Q1 + Q2 * Q2 ;
    } while (Q > 1.0) ;

    return ( std::sqrt(variance) *( std::sqrt( - 2.0 * std::log(Q) / Q) * Q1) );
}

SCA_TDF_MODULE(channel)
{
    public:
    sca_tdf::sca_in<double> in;
    sca_tdf::sca_out<double> out;

    private:
    double attenuation;
    double variance;

    public:
    channel(sc_core::sc_module_name nm,
            double attenuation,
            double variance) : in("in"),
                               out("out"),
                               attenuation(attenuation),
                               variance(variance) {}

    void processing()
    {
        out.write(in.read() * attenuation + gauss_rand(variance));
    }
};

#endif // CHANNEL_H
