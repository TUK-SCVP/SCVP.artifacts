#ifndef BIT_SRC_H
#define BIT_SRC_H

#include <systemc.h>
#include <systemc-ams.h>

SCA_TDF_MODULE(bit_src)
{
    sca_tdf::sca_out<bool> out;

    SCA_CTOR(bit_src) : out("out") {}

    void processing() {
        out.write((bool)(std::rand()%2));
    }
};

#endif // BIT_SRC_H
