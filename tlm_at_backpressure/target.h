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

#ifndef TARGET_H
#define TARGET_H

#include <iomanip>
#include <systemc>
#include <queue>
#include <tlm.h>
#include <tlm_utils/peq_with_cb_and_phase.h>
#include "../tlm_memory_manager/memory_manager.h"
#include "../tlm_protocol_checker/tlm2_base_protocol_checker.h"
#include "../tlm_at_1/util.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

// Internal Phase for transaction processing:
DECLARE_EXTENDED_PHASE(INTERNAL);

class Target: public sc_module, public tlm::tlm_fw_transport_if<>
{
    public:
    tlm::tlm_target_socket<> socket;

    protected:
    bool responseInProgress;
    tlm::tlm_generic_payload* endRequestPending;
    tlm_utils::peq_with_cb_and_phase<Target> peq;
    unsigned int numberOfTransactions;
    unsigned int bufferSize;
    std::queue<tlm::tlm_generic_payload*> responseQueue;

    public:
    SC_HAS_PROCESS(Target);
    Target(sc_module_name name, unsigned int bufferSize = 8) : sc_module(name),
        socket("socket"),
        responseInProgress(false),
        endRequestPending(0),
        peq(this, &Target::peqCallback),
        bufferSize(bufferSize),
        numberOfTransactions(0)
    {
        socket.bind(*this);
    }

    void printBuffer(int max, int n)
    {
        std::cout << "\033[1;35m"
                  << "(T) @"  << setfill(' ') << setw(12) << sc_time_stamp()
                  << " Target Buffer: "
                  << "[";
        for(int i = 0; i < n; i++) {
            std::cout << "█";
        }
        for(int i = 0; i < max-n; i++) {
            std::cout << " ";
        }
        std::cout << "]"
                  << " (Max:" << max << ") "
                  << "\033[0m"
                  << std::endl;
        std::cout.flush();
    }

    virtual void b_transport(tlm::tlm_generic_payload& trans,
                             sc_time& delay)
    {
        executeTransaction(trans);
    }

    // [1.0, 1.6]
    virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans,
                                               tlm::tlm_phase& phase,
                                               sc_time& delay)
    {
        // Queue the transaction into the peq until
        // the annotated time has elapsed
        peq.notify( trans, phase, delay);

        // HINT: Implementation of:
        //       - Return Path Shortcuts [2.0]
        //       - Early Completion [3.0]
        //       - Skip END_REQ [4.0]
        // should be here

        return tlm::TLM_ACCEPTED; // [1.1, 1.7, (1.8)]
    }

    void peqCallback(tlm::tlm_generic_payload& trans,
                     const tlm::tlm_phase& phase)
    {
        sc_time delay;

        if(phase == tlm::BEGIN_REQ) // [1.0]
        {
            // Increment the transaction reference count
            trans.acquire();

            if (numberOfTransactions < bufferSize) // Input buffersize
            {
                sendEndRequest(trans); // [1.2]
                // HINT: instead of [1.2] we can call also [4.1] (ie. [1.4])
            }
            else // If buffer is full we do backpressure
            {
                // Put back-pressure on initiator by deferring END_REQ
                endRequestPending = &trans;
            }
        }
        else if (phase == tlm::END_RESP) // [1.6]
        {
            // On receiving END_RESP, the target can release the transaction
            // and allow other pending transactions to proceed

            if (!responseInProgress)
            {
                SC_REPORT_FATAL("TLM-2",
                   "Illegal transaction phase END_RESP received by target");
            }

            // Reduce number of transactions in target:
            numberOfTransactions--;
            printBuffer(bufferSize, numberOfTransactions);

            // Target itself is now clear to issue the next BEGIN_RESP
            responseInProgress = false;
            if (responseQueue.size() > 0)
            {
                tlm::tlm_generic_payload * next = responseQueue.front();
                responseQueue.pop();
                sendResponse(*next);
            }

            // ... and to unblock the initiator by issuing END_REQ
            if (endRequestPending)
            {
                sendEndRequest(*endRequestPending);
                endRequestPending = 0;
            }
        }
        else if(phase == INTERNAL)
        {
            // Execute the read or write commands
            executeTransaction(trans);

            // Target must honor BEGIN_RESP/END_RESP exclusion rule
            // i.e. must not send BEGIN_RESP until receiving previous
            // END_RESP or BEGIN_REQ
            if (responseInProgress)
            {
                responseQueue.push(&trans);
            }
            else
            {
                sendResponse(trans);
            }
        }
        else // tlm::END_REQ or tlm::BEGIN_RESP
        {
            SC_REPORT_FATAL(name(), "Illegal transaction phase received");
        }
    }

    void sendEndRequest(tlm::tlm_generic_payload& trans)
    {
        tlm::tlm_phase bw_phase;
        sc_time delay;

        // Queue the acceptance and the response with the appropriate latency
        bw_phase = tlm::END_REQ;
        delay = randomDelay(); // Accept delay

        tlm::tlm_sync_enum status;
        status = socket->nb_transport_bw(trans, bw_phase, delay); // [1.2]
        // Ignore return value (has to be TLM_ACCEPTED anyway)
        // initiator cannot terminate transaction at this point

        // Queue internal event to mark beginning of response
        delay = delay + randomDelay(); // Latency
        peq.notify(trans, INTERNAL, delay);

        numberOfTransactions++;
        printBuffer(bufferSize, numberOfTransactions);
    }

    // Common to b_transport and nb_transport
    void executeTransaction(tlm::tlm_generic_payload& trans)
    {
        tlm::tlm_command cmd = trans.get_command();
        sc_dt::uint64    adr = trans.get_address();
        unsigned char*   ptr = trans.get_data_ptr();
        unsigned int     len = trans.get_data_length();
        unsigned char*   byt = trans.get_byte_enable_ptr();
        unsigned int     wid = trans.get_streaming_width();

        if (byt != 0) {
            trans.set_response_status( tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE );
            return;
        }
        if (len > 4 || wid < len) {
            trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
            return;
        }

        if (cmd == tlm::TLM_READ_COMMAND)
        {
            *reinterpret_cast<int*>(ptr) = -int(adr);
        }
        else if (cmd == tlm::TLM_WRITE_COMMAND)
        {
            // Check for expected data
            assert( *reinterpret_cast<unsigned int*>(ptr) == adr );
        }

        cout << "\033[1;32m"
             << "(T) @"  << setfill(' ') << setw(12) << sc_time_stamp()
             << ": " << setw(12) << (cmd ? "Exec. Write " : "Exec. Read ")
             << "Addr = " << setfill('0') << setw(8) << dec << adr
             << " Data = " << "0x" << setfill('0') << setw(8) << hex
             << *reinterpret_cast<int*>(ptr)
             << "\033[0m" << endl;

        trans.set_response_status( tlm::TLM_OK_RESPONSE );
    }

    void sendResponse(tlm::tlm_generic_payload& trans)
    {
        tlm::tlm_sync_enum status;
        tlm::tlm_phase bw_phase;
        sc_time delay;

        responseInProgress = true;
        bw_phase = tlm::BEGIN_RESP;
        delay = SC_ZERO_TIME;
        status = socket->nb_transport_bw( trans, bw_phase, delay ); // [1.4]

        if (status == tlm::TLM_UPDATED) // [2.1]
        {
            // The timing annotation must be honored
            peq.notify( trans, bw_phase, delay);
        }
        else if (status == tlm::TLM_COMPLETED) // [3.1]
        {
            // The initiator has terminated the transaction
            numberOfTransactions--;
            responseInProgress = false;
        }
        // In the case of TLM_ACCEPTED [1.5] we will recv. a FW call [1.6]

        trans.release();
    }

    // TLM-2 forward DMI method
    virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans,
                                    tlm::tlm_dmi& dmi_data)
    {
        // Dummy method
        return false;
    }

    // TLM-2 debug transport method
    virtual unsigned int transport_dbg(tlm::tlm_generic_payload& trans)
    {
        // Dummy method
        return 0;
    }
};

#endif // TARGET_H
