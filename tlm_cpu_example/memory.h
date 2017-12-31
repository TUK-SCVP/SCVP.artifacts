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

#ifndef MEMORY_H
#define MEMORY_H

#include <systemc.h>
#include <tlm.h>

class mem : sc_module, tlm::tlm_fw_transport_if<>
{
    private:
    unsigned char data[1024];

    public:
    tlm::tlm_target_socket<> tSocket;

    SC_CTOR(mem) : tSocket("tSocket")
    {
        tSocket.bind(*this);

        for(int i = 0; i < 1024; i++)
        {
            data[i] = 0;
        }
    }

    void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay)
    {
        if (trans.get_address() >= 1024)
        {
             trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
             return;
        }

        if (trans.get_data_length() != 4)
        {
             trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
             return;
        }

        if(trans.get_command() == tlm::TLM_WRITE_COMMAND)
        {
            memcpy(&data[trans.get_address()], // destination
                   trans.get_data_ptr(),      // source
                   trans.get_data_length()); // size
        }
        else // (trans.get_command() == tlm::TLM_READ_COMMAND)
        {
            memcpy(trans.get_data_ptr(),        // destination
                   &data[trans.get_address()], // source
                   trans.get_data_length());  // size
        }

        delay = delay + sc_time(1, SC_NS);

        trans.set_response_status( tlm::TLM_OK_RESPONSE );
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
    bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans,
                            tlm::tlm_dmi& dmi_data)
    {
        SC_REPORT_FATAL(this->name(),"get_direct_mem_ptr is not implemented");
        return false;
    }

    unsigned int transport_dbg(tlm::tlm_generic_payload& trans)
    {
        if (trans.get_address() >= 1024)
        {
             SC_REPORT_INFO("mem", "Out of memory range");
             return 0;
        }

        if(trans.get_command() == tlm::TLM_WRITE_COMMAND)
        {
            memcpy(&data[trans.get_address()], // destination
                   trans.get_data_ptr(),      // source
                   trans.get_data_length()); // size
        }
        else // (trans.get_command() == tlm::TLM_READ_COMMAND)
        {
            memcpy(trans.get_data_ptr(),        // destination
                   &data[trans.get_address()], // source
                   trans.get_data_length());  // size
        }

        return trans.get_data_length();

    }

};

#endif // MEMORY_H
