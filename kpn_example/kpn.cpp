#include "kpn.h"
#include <unistd.h>

void kpn::kpn_add() // consumes and produces
{
    while(true)
    {
        y.write(a.read() + b.read());
    }
}

void kpn::kpn_a() // produces
{
    while(true)
    {
        wait(sc_time(10,SC_NS));
        a.write(1);
    }
}

void kpn::kpn_b() // produces
{
    while(true)
    {
        wait(sc_time(1,SC_NS));
        b.write(2);
    }
}

void kpn::kpn_y() // consumes
{
    while(true)
    {
        wait(sc_time(20,SC_NS));
        double __attribute__((unused)) value = y.read();
    }
}

// Helping methods:

void kpn::debug_thread()
{
    while(true)
    {
        system("clear");
        std::cout << "Kahn Process Network with SystemC" << std::endl;
        std::cout << "Matthias Jung (2017)" << std::endl;

        std::cout << std::endl << "@" << sc_time_stamp() << std::endl;
        print_fifo(10,10-a.num_free(),"A");
        print_fifo(10,10-b.num_free(),"B");
        print_fifo(20,20-y.num_free(),"Y");

        usleep(300000);

        if(y.num_free()==0)
        {
            sc_stop();
        }
        wait();
    }
}

void kpn::print_fifo(int max, int value, std::string name)
{
    std::cout << name << "[";
    for(int i = 0; i < max; i++)
    {
        if(i<value)
        {
            std::cout << "█";
        }
        else
        {
            std::cout << " ";
        }
    }
    std::cout << "]" << std::endl;
    std::cout.flush();
}
