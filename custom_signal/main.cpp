#include <iostream>
#include <systemc.h>

using namespace std;

template <class T>
class SignalInterface : public sc_interface
{
    public:
    virtual T read() = 0;
    virtual void write(T) = 0;
};

template <class T>
class Signal : public SignalInterface<T>, public sc_prim_channel
{
    private:
    T currentValue;
    T newValue;
    sc_event valueChangedEvent;

    public:
    Signal() {
        currentValue = 0;
        newValue = 0;
    }

    T read()
    {
        return currentValue;
    }

    void write(T d)
    {
        newValue = d;
        if(newValue != currentValue)
        {
            request_update(); // Call to SystemC Scheudler
        }
    }

    void update()
    {
        if(newValue != currentValue)
        {
            currentValue = newValue;
            valueChangedEvent.notify(SC_ZERO_TIME);
        }
    }

    const sc_event& default_event() const {
        return valueChangedEvent;
    }
};

SC_MODULE(PRODUCER)
{
    sc_port< Signal<int> > master;

    SC_CTOR(PRODUCER)
    {
        SC_THREAD(process);
    }

    void process()
    {
        master->write(10);
        wait(10,SC_NS);
        master->write(20);
        wait(20,SC_NS);
        sc_stop();
    }
};

SC_MODULE(CONSUMER)
{
    sc_port< Signal<int> > slave;

    SC_CTOR(CONSUMER)
    {
        SC_METHOD(process);
        sensitive << slave;
        dont_initialize();
    }

    void process()
    {
        int v = slave->read();
        std::cout << v << std::endl;
    }
};

int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    PRODUCER pro1("pro1");
    CONSUMER con1("con1");
    Signal<int> channel;

    pro1.master.bind(channel);
    con1.slave.bind(channel);

    sc_start(sc_time(100,SC_NS));

    return 0;
}
