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

#include <systemc.h>

SC_MODULE(NOT)
{
    public:
    sc_in<bool> in;
    sc_out<bool> out;

    SC_CTOR(NOT) : in("in"), out("out")
    {
        SC_METHOD(process);
    }

    void process()
    {
       out.write(!in.read());
    }
};

SC_MODULE(not_chain)
{
    sc_in<bool> A;
    sc_out<bool> Z;

    NOT not1, not2, not3;

    sc_signal<bool> h1,h2;

    SC_CTOR(not_chain):
        A("A"), Z("Z"),
        not1("not1"), not2("not2"), not3("not3"),
        h1("h1"), h2("h2")
    {
        //        h1    h2
        // A--NOT1--NOT2--NOT3--Z
        not1.in.bind(A);
        not1.out.bind(h1);

        not2.in(h1);
        not2.out(h2);

        not3.in(h2);
        not3.out(Z);
    }
};

int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{
    sc_signal<bool> foo, bar;

    not_chain c("not_chain");

    foo.write(0);
    c.A.bind(foo);
    c.Z.bind(bar);

    sc_start();

    std::cout << bar.read() << std::endl;
    return 0;
}
