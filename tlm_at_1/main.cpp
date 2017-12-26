#include <iomanip>
#include <systemc>
#include "tlm.h"
#include "tlm_utils/peq_with_cb_and_phase.h"
#include "../tlm_memory_manager/memory_manager.h"
#include "../tlm_protocol_checker/tlm2_base_protocol_checker.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

#define LENGTH 2

class Initiator: sc_module, tlm::tlm_bw_transport_if<>
{
    public:
    // TLM-2 socket, defaults to 32-bits wide, base protocol
    tlm::tlm_initiator_socket<> socket;

    private:
    MemoryManager mm;
    int data[16];
    tlm::tlm_generic_payload* requestInProgress;
    sc_event endRequest;
    tlm_utils::peq_with_cb_and_phase<Initiator> peq;

    public:
    SC_CTOR(Initiator)
        : socket("socket")  // Construct and name socket
        , requestInProgress(0)
        , peq(this, &Initiator::peqCallback)
    {
        socket.bind(*this);

        SC_THREAD(process);

        for(int i=0; i<16; i++)
        {
            data[i] = 0;
        }
    }

    private:
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
            delay = sc_time(10, SC_NS);

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
            if (status == tlm::TLM_UPDATED) // [2.0]
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

            wait( sc_time(5, SC_NS) );
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
            sc_time delay = sc_time(5, SC_NS);
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
            char txt[100];
            sprintf(txt,
                    "Transaction returned with error, response status = %s",
                    trans.get_response_string().c_str());
            SC_REPORT_ERROR(name(), txt);
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

class Target: sc_module, tlm::tlm_fw_transport_if<>
{
    public:
    tlm::tlm_target_socket<> socket;

    private:
    tlm::tlm_generic_payload* transactionInProgress;
    sc_event targetDone;
    bool responseInProgress;
    tlm::tlm_generic_payload* nextResponsePending;
    tlm::tlm_generic_payload* endRequestPending;
    tlm_utils::peq_with_cb_and_phase<Target> peq;

    public:
    SC_CTOR(Target) : socket("socket"),
        transactionInProgress(0),
        responseInProgress(false),
        nextResponsePending(0),
        endRequestPending(0),
        peq(this, &Target::peqCallback)
    {
        socket.bind(*this);

        SC_METHOD(executeTransactionProcess);
        sensitive << targetDone;
        dont_initialize();
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

            if (!transactionInProgress)
            {
                sendEndRequest(trans); // [1.2]
                // HINT: instead of [1.2] we can call also [4.1] (ie. [1.4])
            }
            else
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

            // Flag must only be cleared when END_RESP is sent
            transactionInProgress = 0;

            // Target itself is now clear to issue the next BEGIN_RESP
            responseInProgress = false;
            if (nextResponsePending)
            {
                sendResponse(*nextResponsePending);
                nextResponsePending = 0;
            }

            // ... and to unblock the initiator by issuing END_REQ
            if (endRequestPending)
            {
                sendEndRequest(*endRequestPending);
                endRequestPending = 0;
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
        delay = sc_time(20, SC_NS); // Accept delay

        tlm::tlm_sync_enum status;
        status = socket->nb_transport_bw( trans, bw_phase, delay ); // [1.2]
        // Ignore return value (has to be TLM_ACCEPTED anyway)
        // initiator cannot terminate transaction at this point

        // Queue internal event to mark beginning of response
        delay = delay + sc_time(40, SC_NS); // Latency
        targetDone.notify( delay );

        assert(transactionInProgress == 0);
        transactionInProgress = &trans;
    }

    // Method process that runs on targetDone
    void executeTransactionProcess()
    {
        // Execute the read or write commands
        executeTransaction(*transactionInProgress);

        // Target must honor BEGIN_RESP/END_RESP exclusion rule
        // i.e. must not send BEGIN_RESP until receiving previous
        // END_RESP or BEGIN_REQ
        if (responseInProgress)
        {
            // Target allows only two transactions in-flight
            if (nextResponsePending)
            {
                SC_REPORT_FATAL(name(),
                   "Attempt to have two pending responses in target");
            }
            nextResponsePending = transactionInProgress;
        }
        else
        {
            sendResponse(*transactionInProgress);
        }
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
             << "Addr = 0x" << setfill('0') << setw(8) << hex << adr
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
        else if (status == tlm::TLM_COMPLETED)
        {
            SC_REPORT_FATAL(name(),
                            "This transition is deprecated since TLM2.0.1");
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

int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{
    cout << std::endl;

    Initiator* initiator = new Initiator("initiator");
    Target* target = new Target("target");

    tlm_utils::tlm2_base_protocol_checker<> *chk =
        new tlm_utils::tlm2_base_protocol_checker<>("chk");

    // Binding:
    initiator->socket.bind(chk->target_socket);
    chk->initiator_socket.bind(target->socket);

    sc_start();
    return 0;
}
