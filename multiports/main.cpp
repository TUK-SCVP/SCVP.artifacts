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
    sc_port<sc_fifo_out_if<int>,0,SC_ZERO_OR_MORE_BOUND> port;

    SC_CTOR(module)
    {
        SC_THREAD(process);
    }

    void process()
    {
        wait(SC_ZERO_TIME);

        port[0]->write(1);
        std::cout << "Write to port 0" << std::endl;
        wait(1,SC_NS);

        port[1]->write(1);
        std::cout << "Write to port 1" << std::endl;
        wait(1,SC_NS);

        port[2]->write(1);
        std::cout << "Write to port 2" << std::endl;
        wait(1,SC_NS);

        for(int i=0; i < port.size(); i++)
        {
           port[i]->write(2);
           std::cout << "Write to port " << i << std::endl;
           wait(1,SC_NS);
        }
    }

};

int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    module m("m");
    sc_fifo<int> f1, f2, f3;

    m.port.bind(f1);
    m.port.bind(f2);
    m.port.bind(f3);

    sc_start();
    return 0;
}
