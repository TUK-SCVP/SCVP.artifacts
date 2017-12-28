#ifndef UTIL_H
#define UTIL_H
#include <systemc.h>

sc_time randomDelay()
{
    unsigned int nanoseconds = rand()%1000;
    return sc_time(nanoseconds, SC_NS);
}

#endif // UTIL_H
