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


#ifndef KPN_H
#define KPN_H

#include <systemc.h>
#include "utils.h"


// This KPN consists of three Processes and three FIFOs. P3 reads very often
// from f3 and less often from f1, which will create a blocking write situation
// for p1. Therefore p1 won't produce new data and p2 and p3 will be read locked
// after some time. This is an artificial deadlock.

//      +--f1---> p3
//     /          ^
//    /           |
//   /            f3
//  /             |
// p1 ----f2----> p2


SC_MODULE(kpn)
{
  private:

    // Declare FIFOs
    // For debugging purposes we derived our own FIFO class from sc_fifo!
    my_sc_fifo<unsigned int> f1, f2, f3;

    // Declare Processes
    void p1();
    void p2();
    void p3();

    // Internal Variables
    unsigned int counter;
    unsigned int stream1;
    unsigned int stream2;


  public:
    SC_CTOR(kpn) : f1("f1", 5), // Adjust FIFO Sizes Here
                   f2("f2", 5), //        - " -
                   f3("f3", 5)  //        - " -
    {
       SC_THREAD(p1);
       SC_THREAD(p2);
       SC_THREAD(p3);

       stream1 = 0;
       stream2 = 0;
    }

    bool success = false;
};

#endif // KPN_H
