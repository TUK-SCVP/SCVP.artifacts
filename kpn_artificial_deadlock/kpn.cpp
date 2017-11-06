#include "kpn.h"
#include <unistd.h>
#include <utils.h>


void kpn::p1()
{
    unsigned int integers = 0;
    while(true)
    {
        f1.write(integers);
        f2.write(integers);
        integers++;

        //wait(SC_ZERO_TIME);
    }
}

void kpn::p2()
{
    while(true)
    {
        stream1 = f3.read();
        if(stream1 % 10 != 0)
        {
            stream2 = f1.read();
        }

        if(stream1 == 100)
        {
            success = true;
            sc_stop();
        }

        //wait(SC_ZERO_TIME);
    }
}

void kpn::p3()
{
    while(true)
    {
        unsigned int u = f2.read();
        f3.write(u);

        //wait(SC_ZERO_TIME);
    }
}

