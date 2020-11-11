#include <systemc.h>
#include <iostream>
#include "kpn.h"

using namespace std;

int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    kpn kahn("kpn");
    sc_start();
    return 0;
}
