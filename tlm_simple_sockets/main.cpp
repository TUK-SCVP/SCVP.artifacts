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

// PEQ:
#include <tlm_utils/peq_with_cb_and_phase.h>

// MM and tools:
#include "../tlm_at_1/util.h"
#include "../tlm_memory_manager/memory_manager.h"

// Internal Phase for transaction processing:
DECLARE_EXTENDED_PHASE(INTERNAL);

#define N 1024

using namespace std;

SC_MODULE(Initiator)
{
    public:
    // TLM-2 socket, defaults to 32-bits wide, base protocol
    tlm_utils::simple_initiator_socket<Initiator> iSocket;

    protected:
    MemoryManager mm;
    int data[16];
    tlm::tlm_generic_payload* requestInProgress;
    sc_event endRequest;
    tlm_utils::peq_with_cb_and_phase<Initiator> peq;

    public:
    SC_CTOR(Initiator): iSocket("iSocket"),
                        requestInProgress(0),
                        peq(this, &Initiator::peqCallback)
    {
        iSocket.register_nb_transport_bw(this, &Initiator::nb_transport_bw);

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

        // Do b_transports:
        for (int i = 0; i < N; i++)
        {
            tlm::tlm_generic_payload trans;
            data[i % 16] = i;
            trans.set_address(rand()%N);
            trans.set_data_length(4);
            trans.set_streaming_width(4);
            trans.set_command(tlm::TLM_WRITE_COMMAND);
            trans.set_data_ptr(reinterpret_cast<unsigned char*>(&data[i%16]));
            trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

            sc_time delay = randomDelay();

            iSocket->b_transport(trans, delay);

            if (trans.is_response_error())
            {
                SC_REPORT_FATAL(name(), "Response error from b_transport");
            }

            wait(delay);

            cout << "\033[1;31m("
                 << name()
                 << ")@"  << setfill(' ') << setw(12) << sc_time_stamp()
                 << ": " << setw(12) << "Write to "
                 << "Addr = " << setfill('0') << setw(8)
                 << dec << trans.get_address()
                 << " Data = " << "0x" << setfill('0') << setw(8)
                 << hex << data[i%16] << "(b_transport) \033[0m" << endl;
        }

        // Do nb_transports:
        for (int i = 0; i < N; i++)
        {
            int adr = rand()%N;
            tlm::tlm_command cmd = tlm::TLM_READ_COMMAND;

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

            cout << "\033[1;31m("
                 << name()
                 << ")@"  << setfill(' ') << setw(12) << sc_time_stamp()
                 << ": " << setw(12) << (cmd ? "Write to " : "Read from ")
                 << "Addr = " << setfill('0') << setw(8) << dec << adr
                 << " Data = " << "0x" << setfill('0') << setw(8)
                 << hex << data[i%16] << "(nb_transport) \033[0m" << endl;

            // Non-blocking transport call on the forward path
            tlm::tlm_sync_enum status;

            // Call [1.0]:
            status = iSocket->nb_transport_fw( *trans, phase, delay );

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
            sc_time delay = sc_time(10, SC_NS);
            // [1.6]
            iSocket->nb_transport_fw( trans, fw_phase, delay ); // Ignore return

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

        cout << "\033[1;31m("
             << name()
             << ")@"  << setfill(' ') << setw(12) << sc_time_stamp()
             << ": " << setw(12) << (cmd ? "Check Write " : "Check Read ")
             << "Addr = " << setfill('0') << setw(8) << dec << adr
             << " Data = " << "0x" << setfill('0') << setw(8) << hex << *ptr
             << "\033[0m" << endl;
    }
};

SC_MODULE(Target)
{
    public:
    tlm_utils::simple_target_socket<Target> tSocket;

    private:
    unsigned char mem[512];

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
        tSocket("tSocket"),
        responseInProgress(false),
        endRequestPending(0),
        peq(this, &Target::peqCallback),
        bufferSize(bufferSize),
        numberOfTransactions(0)
    {
        tSocket.register_b_transport(this, &Target::b_transport);
        tSocket.register_nb_transport_fw(this, &Target::nb_transport_fw);
    }

    void printBuffer(int max, int n)
    {
        std::cout << "\033[1;35m("
                  << name()
                  << ")@"  << setfill(' ') << setw(12) << sc_time_stamp()
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
                SC_REPORT_FATAL(name(),
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
        status = tSocket->nb_transport_bw(trans, bw_phase, delay); // [1.2]
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


        if (trans.get_address() >= 512) {
            trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
            return;
        }
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
            memcpy(&mem[trans.get_address()], // destination
                   trans.get_data_ptr(),      // source
                   trans.get_data_length());  // size
        }
        else if (cmd == tlm::TLM_WRITE_COMMAND)
        {
            memcpy(trans.get_data_ptr(),      // destination
                   &mem[trans.get_address()], // source
                   trans.get_data_length());  // size
        }

        cout << "\033[1;32m("
             << name()
             << ")@"  << setfill(' ') << setw(12) << sc_time_stamp()
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

        sc_assert(responseInProgress == false);

        responseInProgress = true;
        bw_phase = tlm::BEGIN_RESP;
        delay = SC_ZERO_TIME;
        status = tSocket->nb_transport_bw( trans, bw_phase, delay ); // [1.4]

        if (status == tlm::TLM_UPDATED) // [2.1]
        {
            // The timing annotation must be honored
            peq.notify( trans, bw_phase, delay);
        }
        else if (status == tlm::TLM_COMPLETED)
        {
            SC_REPORT_FATAL(name(),
                            "This transition is deprecated since TLM2.0.1");
        }
        // In the case of TLM_ACCEPTED [1.5] we will recv. a FW call [1.6]

        trans.release();
    }
};

template<unsigned int I, unsigned int T>
SC_MODULE(Interconnect)
{
    public:
    tlm_utils::simple_target_socket_tagged<Interconnect> tSocket[T];
    tlm_utils::simple_initiator_socket_tagged<Interconnect> iSocket[I];

    SC_CTOR(Interconnect)
    {
        for(unsigned int i = 0; i < T; i++)
        {
            //tSocket[i] = new tlm_utils::simple_target_socket_tagged<Interconnect>("tSocket");
            tSocket[i].register_b_transport(this, &Interconnect::b_transport, i);
            tSocket[i].register_nb_transport_fw(this, &Interconnect::nb_transport_fw, i);
        }

        for(unsigned int i = 0; i < I; i++)
        {
            //iSocket[i] = new tlm_utils::simple_initiator_socket_tagged<Interconnect>("iSocket");
            iSocket[i].register_nb_transport_bw(this, &Interconnect::nb_transport_bw, i);
        }
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
        sc_assert(id < T);
        int outPort = routeFW(id, trans, false);
        iSocket[outPort]->b_transport(trans, delay);
    }


    virtual tlm::tlm_sync_enum nb_transport_fw( int id,
                                                tlm::tlm_generic_payload& trans,
                                                tlm::tlm_phase& phase,
                                                sc_time& delay )
    {
        sc_assert(id < T);
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
        sc_assert(id < I);
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

    Interconnect<2,2> * bus = new Interconnect<2,2>("B1");

    cpu1->iSocket.bind(bus->tSocket[0]);
    cpu2->iSocket.bind(bus->tSocket[1]);
    bus->iSocket[0].bind(memory1->tSocket);
    bus->iSocket[1].bind(memory2->tSocket);

    sc_start();

    return 0;
}
