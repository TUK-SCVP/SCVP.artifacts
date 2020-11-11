#ifndef SIGNAL_H
#define SIGNAL_H

#include <systemc.h>

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
#endif // SIGNAL_H
