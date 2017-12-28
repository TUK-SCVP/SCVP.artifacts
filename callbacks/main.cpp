/*
 * Copyright 2017 Matthias Jung
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     - Matthias Jung
 */

#include <iostream>
#include <systemc.h>

using namespace std;

SC_MODULE(module)
{
    public:
    sc_in<bool> clk;
    sc_trace_file *tf;

    SC_CTOR(module)
    {
    }

    void process()
    {
        wait(5);
        sc_stop();
    }

    void before_end_of_elaboration()
    {
        cout << "before_end_of_elaboration" << endl;
        SC_THREAD(process);
        sensitive << clk.pos();
    }

    void end_of_elaboration()
    {
        cout << "end_of_elaboration" << endl;
    }

    void start_of_simulation()
    {
        cout << "start_of_simulation" << endl;
        tf = sc_create_vcd_trace_file("trace");
    }

    void end_of_simulation()
    {
        cout << "end_of_simulation" << endl;
        sc_close_vcd_trace_file(tf);
    }

};


int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    module m("m");
    sc_clock clk("clk",sc_time(1,SC_NS));

    m.clk(clk);

    sc_start();
    return 0;
}
