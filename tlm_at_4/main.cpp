#include <iomanip>
#include <systemc>
#include "tlm.h"
#include "tlm_utils/peq_with_cb_and_phase.h"
#include "../tlm_memory_manager/memory_manager.h"
#include "../tlm_protocol_checker/tlm2_base_protocol_checker.h"
#include "../tlm_at_1/target.h"
#include "../tlm_at_1/initiator.h"
#include "../tlm_at_1/util.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

class SkipTarget: public Target
{
    private:
    tlm_utils::peq_with_cb_and_phase<SkipTarget> peq;

    public:
    SC_HAS_PROCESS(SkipTarget);
    SkipTarget(sc_module_name name) :
        Target(name),
        peq(this, &SkipTarget::peqCallback)
    {
    }

    private:
    virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans,
                                               tlm::tlm_phase& phase,
                                               sc_time& delay)
    {
        if(phase == tlm::BEGIN_REQ)
        {
            // Try the [4.0] way if it is not working go [4.1]
            if(!responseInProgress)
            {
                cout << "\033[1;35m"
                     << "(T) @"  << setfill(' ') << setw(12) << sc_time_stamp()
                     << ": " << "Skip END_REQ [4.0] \033[0m" << endl;
                Target::executeTransaction(trans);
                responseInProgress = true;
                phase = tlm::BEGIN_RESP;
                delay += randomDelay();
                return tlm::TLM_UPDATED;
            }
            else // Going the [4.1] way
            {
                peq.notify( trans, phase, delay);
            }
        }
        else if(phase == tlm::END_RESP) // Normal handshake [1.6]
        {
            peq.notify( trans, phase, delay);
        }
        else // tlm::END_REQ or tlm::BEGIN_RESP
        {
            SC_REPORT_FATAL("Skip Target",
                            "Illegal transaction phase received");
        }
        return tlm::TLM_ACCEPTED; // [1.1, 1.7, (1.8)]
    }

    public:

    void peqCallback(tlm::tlm_generic_payload& trans,
                     const tlm::tlm_phase& phase)
    {
        sc_time delay = SC_ZERO_TIME;

        if(phase == tlm::BEGIN_REQ) // [1.0] going to [4.1]
        {
            // Increment the transaction reference count
            trans.acquire();
            delay += sc_time(15, SC_PS);
            targetDone.notify(delay);
            cout << "\033[1;35m"
               << "(T) @"  << setfill(' ') << setw(12) << sc_time_stamp()
               << ": " << "Skip END_REQ [4.1] \033[0m" << endl;
            transactionInProgress = &trans;
        }
        else if (phase == tlm::END_RESP) // [1.6]
        {
            // On receiving END_RESP, the target can release the transaction
            // and allow other pending transactions to proceed

            if (!responseInProgress)
            {
                SC_REPORT_FATAL("Skip Target",
                   "Illegal transaction phase END_RESP received by target");
            }

            // Target itself is now clear to issue the next BEGIN_RESP
            responseInProgress = false;
            if (nextResponsePending)
            {
                sendResponse(*nextResponsePending);
                nextResponsePending = 0;
            }
        }
        else // tlm::END_REQ or tlm::BEGIN_RESP
        {
            SC_REPORT_FATAL("Skip Target", "Illegal transaction phase received");
        }
    }

};

int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{
    cout << std::endl;

    Initiator* initiator = new Initiator("initiator");
    SkipTarget* target = new SkipTarget("target");

    tlm_utils::tlm2_base_protocol_checker<> *chk =
        new tlm_utils::tlm2_base_protocol_checker<>("chk");

    // Binding:
    initiator->socket.bind(chk->target_socket);
    chk->initiator_socket.bind(target->socket);

    sc_start();
    return 0;
}
