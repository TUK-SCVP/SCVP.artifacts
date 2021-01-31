#include <systemc.h>
#include <systemc-ams.h>

#include "sin_src.h"
#include "bit_src.h"
#include "mixer.h"
#include "channel.h"
#include "rectifier.h"
#include "ltf_filter.h"
#include "sampler.h"

int sc_main(int argc, char* argv[])
{
    sc_core::sc_set_time_resolution(1.0, sc_core::SC_FS);

    sin_src carrier("carrier", 1, 1.0e3, sca_core::sca_time(0.125, SC_MS));
    bit_src data("data");
    mixer mix("mixer");
    channel c("channel", 1, 0.004);
    rectifier rec("rectifier");
    ltf_filter filter("filter", 100);
    sampler s("sampler");

    sca_tdf::sca_signal<double> carrierSignal;
    sc_core::sc_signal<bool> binarySignal;
    sca_tdf::sca_signal<double> modulatedSignal;
    sca_tdf::sca_signal<double> transmittedSignal;
    sca_tdf::sca_signal<double> rectifiedSignal;
    sca_tdf::sca_signal<double> filteredSignal;
    sca_tdf::sca_signal<bool> demodulatedSignal;

    carrier.out(carrierSignal);
    data.out(binarySignal);
    mix.inBinary(binarySignal);
    mix.inCarrier(carrierSignal);
    mix.out(modulatedSignal);
    c.in(modulatedSignal);
    c.out(transmittedSignal);
    rec.in(transmittedSignal);
    rec.out(rectifiedSignal);
    filter.in(rectifiedSignal);
    filter.out(filteredSignal);
    s.in(filteredSignal);
    s.out(demodulatedSignal);

    sca_util::sca_trace_file* tf = sca_util::sca_create_vcd_trace_file("trace.vcd");
    sca_util::sca_trace(tf, carrierSignal, "carrier");
    sca_util::sca_trace(tf, binarySignal, "binary");
    sca_util::sca_trace(tf, modulatedSignal, "modulated");
    sca_util::sca_trace(tf, transmittedSignal, "transmitted");
    sca_util::sca_trace(tf, rectifiedSignal, "rectified");
    sca_util::sca_trace(tf, filteredSignal, "filtered");
    sca_util::sca_trace(tf, demodulatedSignal, "demodulated");
    sc_core::sc_start(1, sc_core::SC_SEC);
    sca_util::sca_close_vcd_trace_file(tf);

    return 0;
}


