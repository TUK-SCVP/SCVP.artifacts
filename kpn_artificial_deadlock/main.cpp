#include <systemc.h>
#include <iostream>
#include <kpn.h>

using namespace std;

int sc_main(int argc, char* argv[])

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
