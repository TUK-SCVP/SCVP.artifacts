#include <iostream>
#include <systemc.h>
#include <queue>

using namespace std;

enum cmd {READ, WRITE};

struct transaction
{
    unsigned int data;
    unsigned int addr;
    cmd command;
};

class transactionInterface : public sc_interface
{
    public:
    virtual void transport(transaction &trans) = 0;
};

SC_MODULE(PRODUCER)
{
    sc_port< transactionInterface > master;

    SC_CTOR(PRODUCER)
    {
        SC_THREAD(process);
    }

    void process()
    {
        // Write:
        for(unsigned int i=0; i < 4; i++)
        {
            wait(1,SC_NS);
            transaction trans;
            trans.addr = i;
            trans.data = rand();
            trans.command = cmd::WRITE;
            master->transport(trans);
        }

        // Read:
        for(unsigned int i=0; i < 4; i++)
        {
            wait(1,SC_NS);
            transaction trans;
            trans.addr = i;
            trans.data = 0;
            trans.command = cmd::READ;
            master->transport(trans);
            cout << trans.data << endl;
        }
    }
};

class CONSUMER : public sc_module, public transactionInterface
{
    private:
    unsigned int memory[1024];

    public:
    SC_CTOR(CONSUMER)
    {
        for(unsigned int i=0; i < 1024; i++)
        {
            memory[i] = 0; // Initialize memory
        }
    }

    void transport(transaction &trans)
    {
        if(trans.command == cmd::WRITE)
        {
            memory[trans.addr] = trans.data;
        }
        else // cmd::READ
        {
            trans.data = memory[trans.addr];
        }
    }
};

int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    PRODUCER cpu("cpu");
    CONSUMER mem("memory");

    cpu.master.bind(mem);

    sc_start();

    return 0;
}
