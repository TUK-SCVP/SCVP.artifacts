#include <iostream>
#include <systemc.h>

using namespace std;

SC_MODULE(eventTester)
{
    sc_event triggerEvent;

    SC_CTOR(eventTester)
    {
        SC_THREAD(triggerProcess);
        SC_METHOD(sensitiveProcess);
        sensitive << triggerEvent;
        dont_initialize();
    }

    void triggerProcess()
    {
       wait(SC_ZERO_TIME);
       triggerEvent.notify(10,SC_NS);
       triggerEvent.notify(20,SC_NS); // Will be ignored
       triggerEvent.notify(30,SC_NS); // Will be ignored
    }

    void sensitiveProcess()
    {
        cout << "EVENT: @" << sc_time_stamp() << endl;
    }
};

SC_MODULE(eventQueueTester)
{
    sc_event_queue triggerEventQueue;

    SC_CTOR(eventQueueTester)
    {
        SC_THREAD(triggerProcess);
        SC_METHOD(sensitiveProcess);
        sensitive << triggerEventQueue;
        dont_initialize();
    }

    void triggerProcess()
    {
       wait(100,SC_NS);
       triggerEventQueue.notify(10,SC_NS);
       triggerEventQueue.notify(20,SC_NS); // Will not be ignored
       triggerEventQueue.notify(40,SC_NS); // Will not be ignored
       triggerEventQueue.notify(30,SC_NS); // Will not be ignored
    }

    void sensitiveProcess()
    {
        cout << "QUEUE: @" << sc_time_stamp() << endl;
    }
};

int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    eventTester et("et");
    eventQueueTester eqt("eqt");
    sc_start();
    return 0;
}
