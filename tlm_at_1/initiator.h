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

#ifndef INITIATOR_H
#define INITIATOR_H
#include <iomanip>
#include <systemc>
#include <tlm.h>
#include <tlm_utils/peq_with_cb_and_phase.h>
#include "../tlm_memory_manager/memory_manager.h"
#include "../tlm_protocol_checker/tlm2_base_protocol_checker.h"
#include "util.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

#define LENGTH 100

class Initiator: public sc_module, public tlm::tlm_bw_transport_if<>
{
    public:
    // TLM-2 socket, defaults to 32-bits wide, base protocol
    tlm::tlm_initiator_socket<> socket;

    protected:
    MemoryManager mm;
    int data[16];
    tlm::tlm_generic_payload* requestInProgress;
    sc_event endRequest;
    tlm_utils::peq_with_cb_and_phase<Initiator> peq;

    public:
    SC_CTOR(Initiator): socket("socket"),
                        requestInProgress(0),
                        peq(this, &Initiator::peqCallback)
    {
        socket.bind(*this);

        SC_THREAD(process);

        for(int i=0; i<16; i++)
        {
            data[i] = 0;
        }
    }

    protected:
    void process()
    {
        tlm::tlm_generic_payload* trans;
        tlm::tlm_phase phase;
        sc_time delay;

        // Generate a sequence of random transactions
        for (int i = 0; i < LENGTH; i++)
        {
            int adr = rand();
            tlm::tlm_command cmd = static_cast<tlm::tlm_command>(rand() % 2);
            if (cmd == tlm::TLM_WRITE_COMMAND) data[i % 16] = adr;

            // Grab a new transaction from the memory manager
            trans = mm.allocate();
            trans->acquire();

            trans->set_command(cmd);
            trans->set_address(adr);
            trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data[i%16]));
            trans->set_data_length(4);
            trans->set_streaming_width(4);
            trans->set_byte_enable_ptr(0);
            trans->set_dmi_allowed(false);
            trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

            // Initiator must follow the BEGIN_REQ/END_REQ exclusion rule:
            if (requestInProgress)
            {
                wait(endRequest);
            }
            requestInProgress = trans;
            phase = tlm::BEGIN_REQ;

            // Timing annot. models processing time of initiator prior to call
            delay = randomDelay();

            cout << "\033[1;31m"
                 << "(I) @"  << setfill(' ') << setw(12) << sc_time_stamp()
                 << ": " << setw(12) << (cmd ? "Write to " : "Read from ")
                 << "Addr = 0x" << setfill('0') << setw(8) << hex << adr
                 << " Data = " << "0x" << setfill('0') << setw(8)
                 << hex << data[i%16] << "\033[0m" << endl;

            // Non-blocking transport call on the forward path
            tlm::tlm_sync_enum status;

            // Call [1.0]:
            status = socket->nb_transport_fw( *trans, phase, delay );

            // Check value returned from nb_transport_fw
            if (status == tlm::TLM_UPDATED) // [2.0] or [4.0]
            {
                // The timing annotation must be honored
                peq.notify(*trans, phase, delay);
            }
            else if (status == tlm::TLM_COMPLETED) // [3.0]
            {
                // The completion of the transaction
                // necessarily ends the BEGIN_REQ phase
                requestInProgress = 0;

                // The target has terminated the transaction
                checkTransaction(*trans);

                // Allow the memory manager to free the transaction object
                trans->release();
            }
            // In the case of TLM_ACCEPTED [1.1] we
            // will recv. a BW call in the future [1.2, 1.4]

            wait(randomDelay());
        }
    }

    // [1.2, 1.4]
    virtual tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans,
                                               tlm::tlm_phase& phase,
                                               sc_time& delay)
    {
        // Queue the transaction into the peq until
        // the annotated time has elapsed
        peq.notify(trans, phase, delay);

        // HINT: a Return Path shortcut can be implemented here [2.1]

        return tlm::TLM_ACCEPTED; // [1.3, 1.5]
    }

    // Payload event queue callback
    void peqCallback(tlm::tlm_generic_payload& trans,
                     const tlm::tlm_phase& phase)
    {
        if (phase == tlm::END_REQ // <-- [1.2, 2.0]
            // or [4.0] --V
                || (&trans == requestInProgress && phase == tlm::BEGIN_RESP))
        {
            // The end of the BEGIN_REQ phase
            requestInProgress = 0;
            endRequest.notify(); // wake up suspended main process
        }
        else if (phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
        {
            SC_REPORT_FATAL(name(), "Illegal transaction phase received");
        }

        if (phase == tlm::BEGIN_RESP) // [1.4]
        {
            checkTransaction(trans);

            // Send final phase transition to target
            tlm::tlm_phase fw_phase = tlm::END_RESP;
            sc_time delay = sc_time(randomDelay());
            // [1.6]
            socket->nb_transport_fw( trans, fw_phase, delay ); // Ignore return

            // Allow the memory manager to free the transaction object
            trans.release();
        }
    }

    // Called on receiving BEGIN_RESP or TLM_COMPLETED
    void checkTransaction(tlm::tlm_generic_payload& trans)
    {
        if (trans.is_response_error())
        {
            SC_REPORT_ERROR(name(), "Transaction returned with error!");
        }

        tlm::tlm_command cmd = trans.get_command();
        sc_dt::uint64    adr = trans.get_address();
        int*             ptr = reinterpret_cast<int*>(trans.get_data_ptr());

        cout << "\033[1;31m"
             << "(I) @"  << setfill(' ') << setw(12) << sc_time_stamp()
             << ": " << setw(12) << (cmd ? "Check Write " : "Check Read ")
             << "Addr = 0x" << setfill('0') << setw(8) << hex << adr
             << " Data = " << "0x" << setfill('0') << setw(8) << hex << *ptr
             << "\033[0m" << endl;

        if (cmd == tlm::TLM_READ_COMMAND) // Check if Target did the right thing
        {
            assert(*ptr == -int(adr));
        }
    }

    // TLM-2 backward DMI method
    virtual void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                           sc_dt::uint64 end_range)
    {
        // Dummy method
    }

};


#endif // INITIATOR_H
