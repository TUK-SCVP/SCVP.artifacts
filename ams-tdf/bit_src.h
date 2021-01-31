#ifndef BIT_SRC_H
#define BIT_SRC_H

#include <systemc.h>
#include <systemc-ams.h>

SC_MODULE(bit_src) {
    sc_core::sc_out<bool> out;

    SC_CTOR(bit_src): out("out")
    {
        SC_THREAD(process);
    }

    void process()
    {
        while(true) {
            bool var = (bool)(std::rand()%2);
            out.write(var);
            sc_core::wait(5, sc_core::SC_MS);
        }
    }
};

#endif // BIT_SRC_H
