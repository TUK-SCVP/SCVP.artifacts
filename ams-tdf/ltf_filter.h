#ifndef LTF_FILTER_H
#define LTF_FILTER_H

#include <systemc.h>
#include <systemc-ams.h>

SCA_TDF_MODULE(ltf_filter)
{
    sca_tdf::sca_in<double> in;
    sca_tdf::sca_out<double> out;
    ltf_filter(sc_core::sc_module_name nm,
               double fc,
               double h0 = 1.0
            ) : in("in"),
                out("out"),
                fc(fc),
                h0(h0) {}

    void initialize() {
        num(0) = 1.0;
        den(0) = 1.0;
        den(1) = 1.0 /( 2.0 * M_PI * fc );
    }

    void processing() {
        out.write( ltf_nd( num, den, in.read(), h0 ) );
    }

    private:
    sca_tdf::sca_ltf_nd ltf_nd; // Laplace transfer function
    sca_util::sca_vector<double> num, den; // numerator and denominator coefficients
    double fc; // 3dB cut-off frequency in Hz
    double h0; // DC gain
};

#endif // LTF_FILTER_H
