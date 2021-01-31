#ifndef MIXER_H
#define MIXER_H

#include <systemc.h>
#include <systemc-ams.h>

SCA_TDF_MODULE(mixer)
{
    public:
    sca_tdf::sca_de::sca_in<bool> inBinary;
    sca_tdf::sca_in<double> inCarrier;
    sca_tdf::sca_out<double> out;

    private:
    unsigned long rate;

    public:
    SCA_CTOR(mixer)
            : inBinary("inBinary"),
              inCarrier("inCarrier"),
              out("out"),
              rate(40) {} // use a carrier data rate of 40

    void set_attributes() {
        inCarrier.set_rate(rate);
        out.set_rate(rate);
    }

    void processing() {
        for(unsigned long i = 0; i < rate; i++) {
            if (inBinary.read()) { // 1
                out.write( inCarrier.read(i), i );
            } else { // 0
                out.write( 0.0, i );
            }
        }
    }
};

#endif // MIXER_H
