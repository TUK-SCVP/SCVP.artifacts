#include <iostream>
#include <systemc.h>
#include <tlm.h>


#include "memory_manager.h"

using namespace std;


int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{
    MemoryManager mm;

    tlm::tlm_generic_payload * trans;

    trans = mm.allocate();

    trans->acquire();
    trans->acquire();

    trans->release();
    trans->acquire();

    trans->release();
    trans->release();

    return 0;
}
