#include <iostream>
#include <string>
#include <systemc.h>

SC_MODULE(module)
{
    bool condition1;
    bool condition2;

    SC_CTOR(module)
    {
        condition1 = true;
        condition2 = true;

        sc_assert(condition1 == true && condition2 == true);

        SC_REPORT_INFO("main","Report Info...");
        SC_REPORT_WARNING("main","Report Warning...");
        try{
            SC_REPORT_ERROR("main","Report Error...");
        }
        catch(sc_exception e){
            std::cout << "do some handling for " << e.what() << std::endl;
        }
        SC_REPORT_FATAL("main","Report Error and Stop...");
    }
};

// OPTIONAL:
void reportHandler(const sc_report &report, const sc_actions &actions)
{
    if (actions & SC_DO_NOTHING)
        return;

    if (actions & SC_DISPLAY || actions & SC_LOG)
    {
        std::ostream& stream = actions & SC_DISPLAY ? std::cout : std::cerr;

        std::string severity;

        switch(report.get_severity())
        {
            case SC_INFO    : severity = "INFO   "; break;
            case SC_WARNING : severity = "WARNING"; break;
            case SC_ERROR   : severity = "ERROR  "; break;
            case SC_FATAL   : severity = "FATAL  "; break;
        }

        stream << report.get_time()
               << " + "  << sc_delta_count() << "δ"
               << " "    << report.get_msg_type()
               << " : [" << severity << "] "
               << ' '    << report.get_msg()
               << " (File: "<< report.get_file_name() <<" Line: "
               << report.get_line_number() << ")"
               << std::endl;
    }

    if (actions & SC_THROW) {
        //std::cerr << "warning: the report handler ignored a SC_THROW action\n";
    } else if (actions & SC_INTERRUPT) {
        //std::cerr << "warning: the report handler ignored a SC_INTERRUPT"
        //          << "action\n";
    } else if (actions & SC_CACHE_REPORT) {
        //std::cerr << "warning: the report handler ignored a SC_CACHE_REPORT"
        //          << "action\n";
    }

    if (actions & SC_STOP)
        sc_stop();

    if (actions & SC_ABORT)
        abort();
}

int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    // Optional: Console otherwise ...
    //sc_report_handler::set_log_file_name("output.log");
    //sc_report_handler::set_actions(SC_INFO, SC_LOG);
    //sc_report_handler::set_actions(SC_WARNING, SC_LOG);

    // Optional: Custom Report handler:
    //sc_core::sc_report_handler::set_handler(reportHandler);

    module m("m");

    sc_start();
    return 0;
}
