#include <iomanip>
#include <systemc>
#include "tlm.h"
#include "tlm_utils/peq_with_cb_and_phase.h"
#include "../tlm_memory_manager/memory_manager.h"
#include "../tlm_protocol_checker/tlm2_base_protocol_checker.h"
#include "../tlm_at_1/target.h"
#include "../tlm_at_1/initiator.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

class EarlyTarget: public Target // According to [3.0]
{
    public:
    SC_HAS_PROCESS(EarlyTarget);
    EarlyTarget(sc_module_name name) : Target(name)
    {
    }

    // [1.0, 1.6]
    tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans,
                                               tlm::tlm_phase& phase,
                                               sc_time& delay)
    {
        // Lets do early completion here (No PEQ!):
        Target::executeTransaction(trans);
        delay += sc_time(40, SC_NS);

        cout << "\033[1;35m"
           << "(T) @"  << setfill(' ') << setw(12) << sc_time_stamp()
           << ": " << "Early Completion \033[0m" << endl;
        return tlm::TLM_COMPLETED; // [3.0]
    }
};

int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{
    cout << std::endl;

    Initiator* initiator = new Initiator("initiator");
    EarlyTarget* target = new EarlyTarget("target");

    tlm_utils::tlm2_base_protocol_checker<> *chk =
        new tlm_utils::tlm2_base_protocol_checker<>("chk");

    // Binding:
    initiator->socket.bind(chk->target_socket);
    chk->initiator_socket.bind(target->socket);

    sc_start();
    return 0;
}
