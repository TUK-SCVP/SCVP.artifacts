#include <systemc.h>
#include <iostream>
#include <kpn.h>

using namespace std;

int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])

{
    kpn kahn("kpn");
    sc_start(1,SC_NS);

    if(kahn.success == true)
    {
        std::cout << "SUCESS" << std::endl;
    }
    else
    {
        std::cout << "ARTIFICIAL DEADLOCK" << std::endl;
    }

    return 0;
}
