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
#include <tlm_utils/multi_passthrough_initiator_socket.h>
#include <tlm_utils/multi_passthrough_target_socket.h>

#include "../tlm_memory_manager/memory_manager.h"
#include "../tlm_multipasstrough_sockets/initiator.h"
#include "../tlm_multipasstrough_sockets/target.h"

using namespace std;

class routingExtension : public tlm::tlm_extension<routingExtension>
{
    private:
    unsigned int inputPortNumber;
    unsigned int outputPortNumber;

    public:
    routingExtension(unsigned int i, unsigned int o) : inputPortNumber(i),
                                                       outputPortNumber(o)
    {
        cout << "\033[1;36m(E"
             << ") @"  << setfill(' ') << setw(12) << sc_time_stamp()
             << ": Extension Created = "
             << "  inPort = " << dec << setfill(' ') << setw(2) << i
             << " outPort = " << dec << setfill(' ') << setw(2) << o
             << "\033[0m" << endl;
    }

    tlm_extension_base* clone() const
    {
        return new routingExtension(inputPortNumber, outputPortNumber);
    }

    void copy_from(const tlm_extension_base& ext)
    {
        const routingExtension& cpyFrom =
                static_cast<const routingExtension&>(ext);
        inputPortNumber = cpyFrom.getInputPortNumber();
        outputPortNumber = cpyFrom.getOutputPortNumber();
    }

    unsigned int getInputPortNumber() const
    {
        return inputPortNumber;
    }

    unsigned int getOutputPortNumber() const
    {
        return outputPortNumber;
    }
};


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
            routingExtension* ext = new routingExtension(inPort, outPort);
            trans.set_auto_extension(ext);
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
            routingExtension *ext = NULL;
            trans.get_extension(ext);
            outPort = ext->getOutputPortNumber();
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
        routingExtension *ext = NULL;
        trans.get_extension(ext);
        int inPort = ext->getInputPortNumber();

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
