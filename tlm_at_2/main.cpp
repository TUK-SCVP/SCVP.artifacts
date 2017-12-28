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

#include "../tlm_memory_manager/memory_manager.h"
#include "../tlm_protocol_checker/tlm2_base_protocol_checker.h"
#include "../tlm_at_1/target.h"
#include "../tlm_at_1/initiator.h"
#include "../tlm_at_1/util.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

class ShortcutTarget: public Target // According to [2.0]
{
    public:
    SC_HAS_PROCESS(ShortcutTarget);
    ShortcutTarget(sc_module_name name) : Target(name)
    {
    }

    // [1.0, 1.6]
    tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans,
                                               tlm::tlm_phase& phase,
                                               sc_time& delay)
    {
        if(phase == tlm::BEGIN_REQ)
        {
            if(transactionInProgress) // Do a normal 4 phase handshake
            {
                peq.notify(trans, phase, delay);
            }
            else // Perform the shortcut [2.0]
            {
                // Increment the transaction reference count:
                trans.acquire();

                phase = tlm::END_REQ;

                // Add an accept delay:
                delay += randomDelay();

                // Queue internal event for begin response
                // with internal delay:
                targetDone.notify(delay + randomDelay());

                transactionInProgress = &trans;

                cout << "\033[1;35m"
                     << "(T) @"  << setfill(' ') << setw(12) << sc_time_stamp()
                     << ": " << "FW Return Path Shortcut \033[0m" << endl;

                return tlm::TLM_UPDATED; // [2.0]
            }
        }
        else if (phase == tlm::END_RESP)
        {
            // Normal 4 phase Handshake:
            peq.notify(trans, phase, delay);
            // Return below [1.1]
        }
        else
        {
            SC_REPORT_FATAL("Shortcut Target", "Illegal phase received");
        }

        return tlm::TLM_ACCEPTED; // [1.1]
    }
};

class ShortcutInitiator: public Initiator // According to [2.1]
{
    public:
    SC_HAS_PROCESS(ShortcutInitiator);
    ShortcutInitiator(sc_module_name name) : Initiator(name)
    {
    }

    tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans,
                                       tlm::tlm_phase& phase,
                                       sc_time& delay)
    {
        if(phase == tlm::END_REQ) // Do normal 4 phase handshake here
        {
            peq.notify(trans, phase, delay);
            return tlm::TLM_ACCEPTED; // [1.3]
        }
        else if(phase == tlm::BEGIN_RESP) // Do Shortcut here [2.1]
        {
            if(&trans == requestInProgress)
            {
                // The end of the BEGIN_REQ phase
                requestInProgress = 0;
                endRequest.notify();
            }

            checkTransaction(trans);

            // Allow the MM to free the transaciton object
            trans.release();

            // Send final phase transition:
            phase = tlm::END_RESP;
            delay += randomDelay();

            cout << "\033[1;35m"
                 << "(I) @"  << setfill(' ') << setw(12) << sc_time_stamp()
                 << ": " << "BW Return Path Shortcut \033[0m" << endl;

            return tlm::TLM_UPDATED;
        }

    }

};

int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{
    cout << std::endl;

    ShortcutInitiator* initiator = new ShortcutInitiator("initiator");
    ShortcutTarget* target = new ShortcutTarget("target");

    tlm_utils::tlm2_base_protocol_checker<> *chk =
        new tlm_utils::tlm2_base_protocol_checker<>("chk");

    // Binding:
    initiator->socket.bind(chk->target_socket);
    chk->initiator_socket.bind(target->socket);

    sc_start();
    return 0;
}
