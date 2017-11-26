#include <iostream>
#include <systemc.h>

using namespace std;

int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{
    sc_bv<2> a = 2;
    sc_bv<2> b = "10";
    std::cout << a << std::endl; // 10
    a = 5;
    std::cout << a << std::endl; // 01 overflow
    a = a | b;
    std::cout << a << std::endl; // 11
    bool c = a.and_reduce();
    std::cout << c << std::endl; // 1

    sc_bv<6> d = "000000";
    d.range(0,3) = "1111";
    std::cout << d << std::endl; // 001111
    std::cout << d(0,3) << std::endl; // 1111
    std::cout << d.range(0,3) << std::endl; // 1111
    std::cout << d[0] << std::endl; // 1

    d = (a, d.range(0,3));
    std::cout << d << std::endl; // 111111

    return 0;
}




