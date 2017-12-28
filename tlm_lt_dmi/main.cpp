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
#include <systemc.h>
#include <tlm.h>

using namespace std;

class exampleInitiator: sc_module, tlm::tlm_bw_transport_if<>
{
    private:
    bool dmi;
    tlm::tlm_dmi dmiData;

    public:
    tlm::tlm_initiator_socket<> iSocket;
    SC_CTOR(exampleInitiator) : iSocket("iSocket"), dmi(false)
    {
        iSocket.bind(*this);
        SC_THREAD(process);
    }

    // Dummy method:
    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                   sc_dt::uint64 end_range)
    {
        dmi = false;
    }

    // Dummy method:
    tlm::tlm_sync_enum nb_transport_bw(
            tlm::tlm_generic_payload& trans,
            tlm::tlm_phase& phase,
            sc_time& delay)
    {
        SC_REPORT_FATAL(this->name(),"nb_transport_bw is not implemented");
        return tlm::TLM_ACCEPTED;
    }

    private:
    void process()
    {
        // Write to memory:
        for (int i = 0; i < 16; i++)
        {
            tlm::tlm_generic_payload trans;
            unsigned char data = rand();
            trans.set_address(i);
            trans.set_data_length(1);
            trans.set_streaming_width(1);
            trans.set_command(tlm::TLM_WRITE_COMMAND);
            trans.set_data_ptr(&data);
            trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
            sc_time delay = sc_time(0, SC_NS);


            // If we got an DMI hint and the DMI is allowed in this range
            if ( dmi == true && i >= dmiData.get_start_address()
                             && i <= dmiData.get_end_address())
            {
                if(trans.get_command() == tlm::TLM_READ_COMMAND
                        && dmiData.is_read_allowed())
                {
                    memcpy(
                        &data,
                        dmiData.get_dmi_ptr() + i - dmiData.get_start_address(),
                        trans.get_data_length()
                    );

                    delay += dmiData.get_read_latency();
                }
                else if(trans.get_command() == tlm::TLM_WRITE_COMMAND
                        && dmiData.is_write_allowed())
                {
                    memcpy(
                        dmiData.get_dmi_ptr() + i- dmiData.get_start_address(),
                        &data,
                        trans.get_data_length()
                    );

                    delay += dmiData.get_write_latency();

                    std::cout << "@" << sc_time_stamp() + delay
                              << " Write Data (dmi): "
                              << (unsigned int)data << std::endl;
                }
            }
            else // Normal b_transport
            {
                iSocket->b_transport(trans, delay);

                std::cout << "@" << sc_time_stamp() + delay
                          << " Write Data (b_transport): "
                          << (unsigned int)data << std::endl;

                if(trans.is_dmi_allowed() == true)
                {
                    dmiData.init(); // Reset DMI descriptor
                    dmi = iSocket->get_direct_mem_ptr(trans, dmiData);
                }
            }

            wait(delay);
        }

        sc_stop();
    }
};

class exampleTarget : sc_module, tlm::tlm_fw_transport_if<>
{
    private:
    unsigned char mem[512];

    public:
    tlm::tlm_target_socket<> tSocket;

    SC_CTOR(exampleTarget) : tSocket("tSocket")
    {
        tSocket.bind(*this);
        SC_THREAD(invalidateProcess);
    }

    void invalidateProcess()
    {
        while(true)
        {
            wait(500, SC_NS);
            tSocket->invalidate_direct_mem_ptr(0,511);
        }
    }

    void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay)
    {
        if (trans.get_address() >= 512)
        {
             trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
             return;
        }

        if (trans.get_data_length() != 1)
        {
             trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
             return;
        }

        if(trans.get_command() == tlm::TLM_WRITE_COMMAND)
        {
            memcpy(&mem[trans.get_address()], // destination
                   trans.get_data_ptr(),      // source
                   trans.get_data_length());  // size
        }
        else // (trans.get_command() == tlm::TLM_READ_COMMAND)
        {
            memcpy(trans.get_data_ptr(),      // destination
                   &mem[trans.get_address()], // source
                   trans.get_data_length());  // size
        }

        delay = delay + sc_time(40, SC_NS);

        trans.set_response_status( tlm::TLM_OK_RESPONSE );

        // Give a hint that DMI is possible:
        trans.set_dmi_allowed( true );
    }

    bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans,
                            tlm::tlm_dmi& dmi_data)
    {
        std::cout << "get_direct_mem_ptr called" << std::endl;
        dmi_data.set_dmi_ptr(mem);
        dmi_data.set_start_address(0);
        dmi_data.set_end_address(511);
        dmi_data.set_read_latency(sc_time(40, SC_NS));
        dmi_data.set_write_latency(sc_time(40, SC_NS));
        dmi_data.allow_read_write();
        return true;
    }


    // Dummy method
    tlm::tlm_sync_enum nb_transport_fw(
            tlm::tlm_generic_payload& trans,
            tlm::tlm_phase& phase,
            sc_time& delay )
    {
        SC_REPORT_FATAL(this->name(),"nb_transport_fw is not implemented");
        return tlm::TLM_ACCEPTED;
    }

    // Dummy method
    unsigned int transport_dbg(tlm::tlm_generic_payload& trans)
    {
        SC_REPORT_FATAL(this->name(),"transport_dbg is not implemented");
        return 0;
    }

};

class exampleInterconnect : sc_module,
                            tlm::tlm_bw_transport_if<>,
                            tlm::tlm_fw_transport_if<>
{
    public:
    tlm::tlm_initiator_socket<> iSocket;
    tlm::tlm_target_socket<> tSocket;

    SC_CTOR(exampleInterconnect)
    {
        tSocket.bind(*this);
        iSocket.bind(*this);
    }

    void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay)
    {
        // Annotate Bus Delay
        delay = delay + sc_time(40, SC_NS);
        iSocket->b_transport(trans, delay);
    }

    bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans,
                            tlm::tlm_dmi& dmi_data)
    {
        bool dmi = iSocket->get_direct_mem_ptr(trans, dmi_data);
        dmi_data.set_read_latency( dmi_data.get_read_latency()
                                   + sc_time(40, SC_NS));

        dmi_data.set_write_latency( dmi_data.get_write_latency()
                                   + sc_time(40, SC_NS));
        return dmi;
    }

    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                   sc_dt::uint64 end_range)
    {
        tSocket->invalidate_direct_mem_ptr(start_range, end_range);
    }

    // Dummy method
    virtual tlm::tlm_sync_enum nb_transport_fw(
            tlm::tlm_generic_payload& trans,
            tlm::tlm_phase& phase,
            sc_time& delay )
    {
        SC_REPORT_FATAL(this->name(),"nb_transport_fw is not implemented");
        return tlm::TLM_ACCEPTED;
    }

    // Dummy method
    unsigned int transport_dbg(tlm::tlm_generic_payload& trans)
    {
        SC_REPORT_FATAL(this->name(),"transport_dbg is not implemented");
        return 0;
    }

    // Dummy method:
    tlm::tlm_sync_enum nb_transport_bw(
            tlm::tlm_generic_payload& trans,
            tlm::tlm_phase& phase,
            sc_time& delay)
    {
        SC_REPORT_FATAL(this->name(),"nb_transport_bw is not implemented");
        return tlm::TLM_ACCEPTED;
    }

};

int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{

    exampleInitiator * cpu    = new exampleInitiator("cpu");
    exampleTarget * memory    = new exampleTarget("memory");
    exampleInterconnect * bus = new exampleInterconnect("bus");

    cpu->iSocket.bind(bus->tSocket);
    bus->iSocket.bind(memory->tSocket);

    sc_start();

    return 0;
}
