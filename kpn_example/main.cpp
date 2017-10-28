#include <systemc.h>
#include <iostream>
#include <kpn.h>

using namespace std;

int sc_main(int argc, char* argv[])

{
    kpn kahn("kpn");
    sc_start();
    return 0;
}
