#ifndef RECTIFIER_H
#define RECTIFIER_H

#include <systemc.h>
#include <systemc-ams.h>

SCA_TDF_MODULE(rectifier) {
    sca_tdf::sca_in<double> in;
    sca_tdf::sca_out<double> out;

    SCA_CTOR(rectifier) : in("in"), out("out") {}

    void processing() {
        out.write( std::abs(in.read()) );
    }
};

#endif // RECTIFIER_H
