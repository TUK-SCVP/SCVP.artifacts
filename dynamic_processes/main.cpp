#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <iostream>
#include <systemc.h>

using namespace std;

SC_MODULE(module)
{
    SC_CTOR(module)
    {
        SC_THREAD(parentProcess);
    }

    void parentProcess()
    {
        wait(10,SC_NS);

        cout << "Parent creates Child" << endl;

        sc_process_handle handle = sc_spawn(
            sc_bind(&module::childProcess, this, 5 /* argument */)
        );

        wait(handle.terminated_event());
        cout << "@" << sc_time_stamp() << " End" << endl;
    }

    void childProcess(int id /* 5 */)
    {
        cout << "@" << sc_time_stamp() << " Child " << id
             << " started" << endl;
        wait(10,SC_NS);
    }
};


int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    module m("m");
    sc_start();
    return 0;
}
