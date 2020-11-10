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
#include <iomanip>
#include <systemc.h>
#include <tlm.h>
#include <map>
#include <queue>

// Convenience Sockets:
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/multi_passthrough_initiator_socket.h>
#include <tlm_utils/multi_passthrough_target_socket.h>

// PEQ:
#include <tlm_utils/peq_with_cb_and_phase.h>

// MM and tools:
#include "../tlm_at_1/util.h"
#include "../tlm_memory_manager/memory_manager.h"

// Our local modules
#include "initiator.h"
#include "target.h"

using namespace std;

SC_MODULE(Interconnect)
{
    public:
    tlm_utils::multi_passthrough_target_socket<Interconnect> tSocket;
    tlm_utils::multi_passthrough_initiator_socket<Interconnect> iSocket;

    SC_CTOR(Interconnect) : tSocket("tSocket"), iSocket("iSocket")
    {
        tSocket.register_b_transport(this, &Interconnect::b_transport);
        tSocket.register_nb_transport_fw(this, &Interconnect::nb_transport_fw);
        iSocket.register_nb_transport_bw(this, &Interconnect::nb_transport_bw);
    }

    private:
    std::map<tlm::tlm_generic_payload*, int> bwRoutingTable;
    std::map<tlm::tlm_generic_payload*, int> fwRoutingTable;

    // |----- BEGIN REQ ====>|                     | FW
    // |                     |----- BEGIN REQ ---->|
    // |                     |<==== END REQ -------| BW
    // |<---- END REQ -------|                     |
    // |                     |<==== BEGIN RESP ----| BW
    // |<---- BEGIN RESP ----|                     |
    // |----- END RESP =====>|                     |
    // |                     |----- END RESP ----->| FW

    int routeFW(int inPort,
                tlm::tlm_generic_payload &trans,
                bool store)
    {
        int outPort = 0;

        // Memory map implementation:
        if(trans.get_address() < 512)
        {
            outPort = 0;
        }
        else if(trans.get_address() >= 512 && trans.get_address() < 1024)
        {
            // Correct Address:
            trans.set_address(trans.get_address() - 512);
            outPort = 1;
        }
        else
        {
            trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
        }

        if(store)
        {
            bwRoutingTable[&trans] = inPort;  // From where it comes
            fwRoutingTable[&trans] = outPort; // Where it should go
        }

        return outPort;
    }

    virtual void b_transport( int id,
                              tlm::tlm_generic_payload& trans,
                              sc_time& delay )
    {
        int outPort = routeFW(id, trans, false);
        iSocket[outPort]->b_transport(trans, delay);
    }


    virtual tlm::tlm_sync_enum nb_transport_fw( int id,
                                                tlm::tlm_generic_payload& trans,
                                                tlm::tlm_phase& phase,
                                                sc_time& delay )
    {
        int outPort = 0;

        if(phase == tlm::BEGIN_REQ)
        {
            // In the case of nb_transport_fw the address attribute is valid
            // immediately upon entering the function but only when the phase
            // is BEGIN_REQ. Following the return from any forward path TLM-2.0
            // interface method call, the address attribute will have the value
            // set by the interconnect component lying furthest downstream, and
            // so should be regarded as being undefined for the purposes of
            // transaction routing.
            trans.acquire();

            // Modify address accoring to memory map:
            outPort = routeFW(id, trans, true);
        }
        else if(phase == tlm::END_RESP)
        {
            // Adress was already modified in BEGIN_REQ phase:
            outPort = fwRoutingTable[&trans];
            trans.release();
        }
        else
        {
            SC_REPORT_FATAL(name(),"Illegal phase received by initiator");
        }

        cout << "\033[1;37m("
             << name()
             << ")@"  << setfill(' ') << setw(12) << sc_time_stamp()
             << ": Addr = " << setfill('0') << setw(8)
             << dec << trans.get_address()
             << "  inPort = " << dec << setfill(' ') << setw(2) << id
             << " outPort = " << dec << setfill(' ') << setw(2) << outPort
             << " ptr = " << &trans
             << "\033[0m" << endl;


        return iSocket[outPort]->nb_transport_fw(trans, phase, delay);
    }


    virtual tlm::tlm_sync_enum nb_transport_bw( int id,
                                                tlm::tlm_generic_payload& trans,
                                                tlm::tlm_phase& phase,
                                                sc_time& delay )
    {
        sc_assert(id == fwRoutingTable[&trans]);

        int inPort = bwRoutingTable[&trans];

        return tSocket[inPort]->nb_transport_bw(trans, phase, delay);
    }
};

int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{

    Initiator * cpu1   = new Initiator("C1");
    Initiator * cpu2   = new Initiator("C2");

    Target * memory1   = new Target("M1");
    Target * memory2   = new Target("M2");

    Interconnect * bus = new Interconnect("B1");

    cpu1->iSocket.bind(bus->tSocket);
    cpu2->iSocket.bind(bus->tSocket);
    bus->iSocket.bind(memory1->tSocket);
    bus->iSocket.bind(memory2->tSocket);

    sc_start();

    return 0;
}
