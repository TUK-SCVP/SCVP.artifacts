#include <iostream>
#include <systemc.h>

using namespace std;

SC_MODULE (sc_mutex_example) {
    sc_in<bool> clock;

    sc_mutex bus;

    void busAccess(int who) {
        cout << "@" << sc_time_stamp() <<" Bus access by " << who << endl;
    }

    void process0() {
        while (true) {
            wait();
            cout << "@" << sc_time_stamp() << " Checking mutex 0" << endl;
            // Check if mutex is available
            if (bus.trylock() != -1) {
                cout << "@" << sc_time_stamp() << " Got mutex for 0" << endl;
                busAccess(0);
                wait(2); // Wait for two cycles
                // Unlock the mutex
                cout << "@" << sc_time_stamp() << " 0 mutex unlocked" << endl;
                bus.unlock();
            }
        }
    }

    void process1() {
        while (true) {
            wait();
            cout << "@" << sc_time_stamp() <<" Checking mutex 1"<<endl;
            // Wait till mutex is available
            bus.lock();
            cout << "@" << sc_time_stamp() <<" Got mutex for 1"<<endl;
            busAccess(1);
            wait(3); // Wait for three cycles
            // Unlock the mutex
            cout << "@" << sc_time_stamp() << " 1 mutex unlocked" << endl;
            bus.unlock();
        }
    }

    SC_CTOR(sc_mutex_example) {
        SC_THREAD(process0);
        sensitive << clock;
        SC_THREAD(process1);
        sensitive << clock;
    }
};


int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    sc_clock clock ("my_clock",sc_time(1,SC_NS));

    sc_mutex_example object("wait");
    object.clock (clock);

    sc_start(10,SC_NS);
    return 0;
}
