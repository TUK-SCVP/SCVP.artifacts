/*
 * Copyright 2017 Matthias Jung
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     - Matthias Jung
 */

#include <iostream>
#include <systemc.h>
#include "../custom_signal/signal.h"

template <>
class SignalInterface<bool> : public sc_interface
{
    public:
    virtual bool read() = 0;
    virtual void write(bool) = 0;
    virtual const sc_event & getPosEvent() const = 0;
    virtual const sc_event & getNegEvent() const = 0;
};

template<>
class Signal <bool> : public SignalInterface<bool>, public sc_prim_channel
{
  private:
    bool currentValue;
    bool newValue;
    sc_event valueChangedEvent;
    sc_event posEvent;
    sc_event negEvent;

  public:
    Signal() {
        currentValue = 0;
        newValue = 0;
    }

    bool read()
    {
        return currentValue;
    }

    void write(bool d)
    {
        newValue = d;
        if(newValue != currentValue)
        {
            request_update(); // Call to SystemC Scheudler
        }
    }

    const sc_event& default_event() const {
        return valueChangedEvent;
    }

    void update()
    {
        if(newValue != currentValue)
        {
            currentValue = newValue;
            if(currentValue == false)
            {
                negEvent.notify(SC_ZERO_TIME);
            }
            else // (currentValue == false)
            {
                posEvent.notify(SC_ZERO_TIME);
            }

            valueChangedEvent.notify(SC_ZERO_TIME);
        }
    }

    const sc_event & getPosEvent() const
    {
        return posEvent;
    }

    const sc_event & getNegEvent() const
    {
        return posEvent;
    }

};

class BoolPort : public sc_port<SignalInterface<bool>>
{
    public:
    sc_event_finder & pos()
    {
        return *new sc_event_finder_t<SignalInterface<bool>>(
            *this, &SignalInterface<bool>::getPosEvent);
    }

    sc_event_finder & neg()
    {
        return *new sc_event_finder_t<SignalInterface<bool>>(
            *this, &SignalInterface<bool>::getNegEvent);
    }
};

SC_MODULE(PRODUCER)
{
    BoolPort master;

    SC_CTOR(PRODUCER)
    {
        SC_THREAD(process);
    }

    void process()
    {
        master->write(true);
        wait(10,SC_NS);
        master->write(false);
        wait(10,SC_NS);
        master->write(true);
        wait(10,SC_NS);
        master->write(false);
        wait(10,SC_NS);
        master->write(true);
        wait(10,SC_NS);
        master->write(false);
        wait(10,SC_NS);
        sc_stop();
    }
};

SC_MODULE(CONSUMER)
{
    BoolPort slave;

    SC_CTOR(CONSUMER)
    {
        SC_METHOD(process);
        //sensitive << slave->getPosEvent();
        sensitive << slave.pos();
        dont_initialize();
    }

    void process()
    {
        bool v = slave->read();
        std::cout << v << std::endl;
    }
};

int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    PRODUCER pro1("pro1");
    CONSUMER con1("con1");
    Signal<bool> channel;

    pro1.master.bind(channel);
    con1.slave.bind(channel);

    sc_start(sc_time(100,SC_NS));

    return 0;
}
