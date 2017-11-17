#include <iostream>
#include <systemc.h>

using namespace std;

template <class T>
class myInterface : public sc_interface
{
    public:
    virtual bool read(T&) = 0;
    virtual bool write(T&) = 0;
};

template <class T>
class myChannel : public myInterface<T>//, public sc_prim_channel
{
    private:
    bool dataAvailable;
    T data;

    public:
    myChannel()
    {
        data = 0;
        dataAvailable = false;
    }

    bool read(T &d)
    {
        if(dataAvailable)
        {
            d = data;
            dataAvailable = false;
            return true;
        }
        return false;
    }

    bool write(T &d)
    {
        if(dataAvailable)
        {
            return false;
        }

        data = d;
        dataAvailable = true;
        return true;
    }
};

SC_MODULE(PRODUCER)
{
    sc_port< myInterface<int> > master;

    SC_CTOR(PRODUCER)
    {
        SC_THREAD(process);
    }

    void process()
    {
        int counter = 0;
        while(true)
        {
            if(master->write(counter) == true)
            {
                std::cout << "@" << sc_time_stamp()
                          << ": WRITE=" << counter << std::endl;
                counter++;
            }
            else
            {
                std::cout << "@" << sc_time_stamp()
                          << ": WRITE=NOTHING" << std::endl;
            }

            // wait a random time
            wait(rand() % 10 + 1, SC_NS);
        }
    }
};

SC_MODULE(CONSUMER)
{
    sc_port< myInterface<int> > slave;

    SC_CTOR(CONSUMER)
    {
        SC_THREAD(process);
    }

    void process()
    {
        int counter = 0;
        while(true)
        {
            if(slave->read(counter) == true)
            {
                std::cout << "@" << sc_time_stamp()
                          << ": READ=" << counter << std::endl;
            }
            else
            {
                std::cout << "@" << sc_time_stamp()
                          << ": READ=NOTHING" << std::endl;
            }

            // wait a random time
            wait(rand() % 10 + 1, SC_NS);
        }
    }
};

int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    PRODUCER pro1("pro1");
    CONSUMER con1("con1");
    myChannel<int> ch1;

    pro1.master.bind(ch1);
    con1.slave.bind(ch1);

    sc_start(sc_time(100,SC_NS));

    return 0;
}
