#ifndef INITIATOR_H
#define INITIATOR_H

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


#endif // INITIATOR_H
