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
#include <queue>

using namespace std;

template <class T>
class SimpleFIFOInterface : public sc_interface
{
    public:
    virtual T read() = 0;
    virtual void write(T) = 0;
    // Just for Debug
    virtual void printFIFO()
    {
        cout << "Warning: Debug Function Not Implemented" << endl;
    }
};

template <class T>
class SimpleFIFO : public SimpleFIFOInterface<T>
{
    private:
    std::queue<T> fifo;
    sc_event writtenEvent;
    sc_event readEvent;
    unsigned int maxSize;

    public:
    SimpleFIFO(unsigned int size=16) : maxSize(size)
    {
    }

    T read()
    {
        if(fifo.empty() == true)
        {
            std::cout << "Wait for Write" << std::endl;
            wait(writtenEvent);
        }
        T val = fifo.front();
        fifo.pop();
        readEvent.notify(SC_ZERO_TIME);
        return val;
    }

    void write(T d)
    {
        if(fifo.size() == maxSize)
        {
            std::cout << "Wait for Read" << std::endl;
            wait(readEvent);
        }
        fifo.push(d);
        writtenEvent.notify(SC_ZERO_TIME);
    }

    void printFIFO()
    {
        unsigned int n = fifo.size();

        std::cout << "SimpleFIFO (" << maxSize << ") " << "[";
        for(unsigned int i = 0; i < n; i++) {
            std::cout << "█";
        }
        for(unsigned int i = 0; i < maxSize-n; i++) {
            std::cout << " ";
        }
        std::cout << "]" << std::endl;
        std::cout.flush();
    }
};

SC_MODULE(PRODUCER)
{
    sc_port< SimpleFIFOInterface<int> > master;

    SC_CTOR(PRODUCER)
    {
        SC_THREAD(process);
    }

    void process()
    {
        while(true)
        {
            wait(1,SC_NS);
            master->write(10);
            std::cout << "@" << sc_time_stamp() << " Write: 10 ";
            master->printFIFO();
        }
    }
};

SC_MODULE(CONSUMER)
{
    sc_port< SimpleFIFOInterface<int> > slave;

    SC_CTOR(CONSUMER)
    {
        SC_THREAD(process);
        sensitive << slave;
    }

    void process()
    {
        while(true)
        {
            wait(4,SC_NS);
            std::cout << "@" << sc_time_stamp() << " Read : "
                      << slave->read() << " ";
            slave->printFIFO();
        }
    }
};

int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    PRODUCER pro1("pro1");
    CONSUMER con1("con1");
    SimpleFIFO<int> channel(4);

    sc_signal<int> foo;

    pro1.master.bind(channel);
    con1.slave.bind(channel);

    sc_start(10,SC_NS);

    return 0;
}
