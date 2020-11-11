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

SC_MODULE(toplevel)
{
  public:
    sc_signal<int> A;
    sc_signal<int> B;

    sc_signal<int> C;
    sc_signal<int> D;

    int E;
    int F;

    sc_event startMethod1;
    sc_event startMethod2;
    sc_event startThread1;

    sc_time currentTime;
    unsigned long long currentDelta;

    SC_CTOR(toplevel) : A("A"), B("B")
    {
        SC_METHOD(method1);

        // With initialization: method is called at the beginning of the simulation
        sensitive << startMethod1;

        SC_THREAD(thread1); // Without initialization: thread is not called at the beginning of the simulation
        dont_initialize();
        sensitive << startThread1;

        SC_METHOD(method2); // Without initialization
        dont_initialize();
        sensitive << startMethod2;

        SC_THREAD(controlThread);

        A.write(1);
        B.write(2);
        C.write(3);
        D.write(4);
        E=5;
        F=6;

        currentTime = SC_ZERO_TIME;
        currentDelta = sc_delta_count();
    }

    void waitAndPrint(sc_time delay)
    {
        wait(delay);
        sc_time time = sc_time_stamp();

        // The sc_delta_count() returns the total number of executed
        // delta delays. In order to estimate the delta delay
        // between the time advances the following is done:
        if(time > currentTime)
        {
           currentDelta = sc_delta_count();
           currentTime = time;
        }

        unsigned long long delta = sc_delta_count() - currentDelta;

        std::cout << time <<" + " << delta << "δ\t"
                  << A.read() << "\t"
                  << B.read() << "\t"
                  << C.read() << "\t"
                  << D.read() << "\t"
                  << E << "\t"
                  << F << "\t"
                  << std::endl;
    }

    void controlThread()
    {
        // Start
        waitAndPrint(SC_ZERO_TIME);

        // Set:
        std::cout << "SWAP A<->B" << std::endl;
        startMethod1.notify();
        waitAndPrint(SC_ZERO_TIME);

        std::cout << "SWAP A<->B" << std::endl;
        startMethod1.notify();
        waitAndPrint(SC_ZERO_TIME);

        std::cout << "SWAP C<->D" << std::endl;
        startThread1.notify();
        waitAndPrint(SC_ZERO_TIME);

        std::cout << "SWAP C<->D Again" << std::endl;
        startThread1.notify();
        waitAndPrint(SC_ZERO_TIME);

        std::cout << "SWAP E<->F" << std::endl;
        startMethod2.notify();
        waitAndPrint(SC_ZERO_TIME);


        sc_stop();
    }

    void method1()
    {
        // Using signals:
        A.write(B.read()); // OR: A = B;
        B.write(A.read()); // OR: B = A;
    }

    void thread1()
    {
        // Using signals:
        C = D; // OR: C.write(D.read());
        D = C; // OR: D.write(C.read());
    }

    void method2()
    {
        // Using normal C variables not signals!
        E = F;
        F = E;
    }
};

int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{
    std::cout << "\nT\t\tA\tB\tC\tD\tE\tF" << std::endl;
    toplevel t("t");

    sc_set_stop_mode(SC_STOP_FINISH_DELTA);
    sc_start();

    return 0;
}
