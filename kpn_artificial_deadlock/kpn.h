#ifndef KPN_H
#define KPN_H

#include <systemc.h>
#include <utils.h>


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
