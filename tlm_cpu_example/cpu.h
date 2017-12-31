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

#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <iomanip>
#include <systemc.h>
#include <tlm.h>

//#define DEBUG

class registers
{
    private:
    int32_t reg[32];
    uint32_t pc;

    public:
    registers()
    {
        for(int i = 0; i < 32; i++ )
        {
            reg[i] = 0;
        }
        pc = 0;
    }

    void set(uint32_t n, int32_t data)
    {
        sc_assert(n > 0 && n < 32);
        reg[n] = data;
    }

    int32_t get(uint32_t n)
    {
        sc_assert(n >= 0 && n < 32);
        return reg[n];
    }

    uint32_t getPc()
    {
        return pc;
    }

    void setPc(uint32_t val)
    {
        sc_assert(val >= 0);
        pc = val;
    }

    void incrementPc()
    {
        pc += 4;
    }
};

// TinyRV1
// - ADD, ADDI, MUL
// - LW, SW
// - JAL, JR
// - BNE

enum CMD
{
    nop,
    add,
    addi,
    mul,
    lw,
    sw,
    jal,
    jr,
    bne
};

enum OPCODE : uint32_t
{
    op_add  = 0b0110011,
    op_addi = 0b0010011,
    op_mul  = 0b0110011,
    op_lw   = 0b0000011,
    op_sw   = 0b0100011,
    op_jal  = 0b1101111,
    op_jr   = 0b1100111,
    op_bne  = 0b1100011
};

enum FUNCT3 : uint32_t
{
    f3_add  = 0b000,
    f3_addi = 0b000,
    f3_mul  = 0b000,
    f3_lw   = 0b010,
    f3_sw   = 0b010,
    f3_jr   = 0b000,
    f3_bne  = 0b001
};

enum FUNCT7 : uint32_t
{
    f7_add = 0b0000000,
    f7_mul = 0b0000001,
};

class cpu: sc_module, tlm::tlm_bw_transport_if<>
{
    public:
    tlm::tlm_initiator_socket<> iSocket;
    SC_CTOR(cpu) : iSocket("iSocket"),
                   cycleTime(sc_time(1, SC_NS)),
                   nopCounter(0)
    {
        iSocket.bind(*this);
        SC_THREAD(process);
    }


    // Dummy method:
    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                   sc_dt::uint64 end_range)
    {
        SC_REPORT_FATAL(this->name(),"invalidate_direct_mem_ptr not implement");
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

    registers r;
    sc_time cycleTime;
    uint8_t nopCounter;

    void dumpMemory(unsigned int size)
    {
        std::cout << std::endl;
        std::cout << "Dump Memory Content of Target:" << std::endl;
        std::cout << std::endl;
        unsigned char * buffer = new unsigned char[size];

        tlm::tlm_generic_payload trans;
        trans.set_address(0);
        trans.set_read();
        trans.set_data_length(size);
        trans.set_data_ptr(buffer);

        unsigned int n = iSocket->transport_dbg(trans);

        for(unsigned int i = 0; i < n; i++)
        {
            std::cout << std::hex
                      << std::setfill('0')
                      << std::setw(2)
                      << (unsigned int)buffer[i];

            if((i+1)%4 == 0)
            {
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;

        delete [] buffer;
    }

    void dumpRegisters()
    {
        cout << endl << "Register Dump:" << endl;
        cout << "PC = " << r.getPc() << endl;

        for(int i = 0; i < 32; i++)
        {
            if(r.get(i) != 0)
            {
                cout << "x" << i << " = " << r.get(i) << endl;
            }
        }
    }

    void initialize()
    {
        FILE * file;
        long size;
        unsigned char * buffer;
        size_t result;

        file = fopen ("test.asm.bin" , "rb");

        if (file == NULL)
        {
            SC_REPORT_FATAL(name(), "Cannot load program file");
        }

        // Obtain file size:
        fseek (file , 0 , SEEK_END);
        size = ftell (file);
        rewind (file);

        // Allocate memory to contain the whole file:
        buffer = (unsigned char*) malloc (sizeof(unsigned char)*size);
        if (buffer == NULL)
        {
            SC_REPORT_FATAL(name(), "Malloc error");
        }

        // Copy the file into the buffer:
        result = fread (buffer, 1, size, file);
        if (result != size)
        {
            SC_REPORT_FATAL(name(), "Reading error");
        }

        tlm::tlm_generic_payload trans;
        trans.set_address(0);
        trans.set_write();
        trans.set_data_length(size);
        trans.set_data_ptr(buffer);

        unsigned int n = iSocket->transport_dbg(trans);

        sc_assert(n == size);

        // terminate
        fclose(file);
        free(buffer);
    }

    void do_b_transport(uint64_t addr,
                       tlm::tlm_command cmd,
                       unsigned char * ptr,
                       sc_time &delay)
    {
        tlm::tlm_generic_payload trans;
        trans.set_address(addr);
        trans.set_data_length(4);
        trans.set_streaming_width(4);
        trans.set_command(cmd);
        trans.set_data_ptr(ptr);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

        iSocket->b_transport(trans, delay);

        if (trans.is_response_error())
        {
            SC_REPORT_FATAL(name(), "Response error from b_transport");
        }
    }

    void process()
    {
        //wait();
        initialize();

        while(1)
        {
            sc_time delay = SC_ZERO_TIME;

            uint32_t inst = 0;

            delay += fetch(inst);

            CMD cmd = decode(inst);

            delay += execute_writeback(cmd, inst);

            wait(delay);
        }
    }

    sc_time fetch(uint32_t &data)
    {
        sc_time delay = sc_time(0, SC_NS);

        do_b_transport(r.getPc(),
                       tlm::TLM_READ_COMMAND,
                       reinterpret_cast<unsigned char*>(&data),
                       delay);

        return delay;
    }

    CMD decode(uint32_t &data)
    {
        // Get opcode:
        uint32_t opcode = (data & 0b00000000000000000000000001111111);
        uint32_t funct3 = (data & 0b00000000000000000111000000000000) >> 12;
        uint32_t funct7 = (data & 0b11111110000000000000000000000000) >> 25;

        if (data == 0)
        {
            return nop;
        }
        else if(opcode == op_add && funct3 == f3_add && funct7 == f7_add)
        {
            return add;
        }
        else if(opcode == op_addi && funct3 == f3_addi)
        {
            return addi;
        }
        else if(opcode == op_mul && funct3 == f3_mul && funct7 == f7_mul)
        {
            return mul;
        }
        else if(opcode == op_lw && funct3 == f3_lw)
        {
            return lw;
        }
        else if(opcode == op_sw && funct3 == f3_sw)
        {
            return sw;
        }
        else if(opcode == op_jal)
        {
            return jal;
        }
        else if(opcode == op_jr && funct3 == f3_jr)
        {
            return jr;
        }
        else if(opcode == op_bne && funct3 == f3_bne)
        {
            return bne;
        }
        else
        {
            SC_REPORT_FATAL(name(), "Instruction not supported by TinyRV1");
        }
    }

    sc_time execute_writeback(CMD cmd, uint32_t &data)
    {
        // Extract registers and immediates common for all instructions:
        sc_time delay = SC_ZERO_TIME;

        uint32_t rs2 = (data & 0b00000001111100000000000000000000) >> 20;
        uint32_t rs1 = (data & 0b00000000000011111000000000000000) >> 15;
        uint32_t rd  = (data & 0b00000000000000000000111110000000) >> 7;

        // Immediate sign extension (imm may be overwritten by specific instr.):
        int32_t imm  = (((int32_t)data)
                             & 0b11111111111100000000000000000000) >> 20;

        bool sign = (data & 0b10000000000000000000000000000000) >> 31;

        if(sign == true)
        {
            imm = imm | 0b11111111111111111111000000000000;
        }

        // Execute instructions
        if(cmd == nop)
        {
            nopCounter++;
            if(nopCounter > 10)
            {
                dumpRegisters();
                sc_stop();
            }

            r.incrementPc();
            return cycleTime;
        }
        else if(cmd == add)
        {
            r.set(rd, r.get(rs1) + r.get(rs2));

            #ifdef DEBUG
            cout << "@" << sc_time_stamp() << " ";
            cout << "x" << rd << " = x" << rs1 << " + x" << rs2 << endl;
            #endif

            r.incrementPc();
            return cycleTime;
        }
        else if (cmd == addi)
        {
            r.set(rd, r.get(rs1) + imm);

            #ifdef DEBUG
            cout << "@" << sc_time_stamp() << " ";
            cout << "x" << rd << " = x" << rs1 << " + " << imm << endl;
            #endif

            r.incrementPc();
            return cycleTime;
        }
        else if(cmd == mul)
        {
            r.set(rd, r.get(rs1) * r.get(rs2));

            #ifdef DEBUG
            cout << "@" << sc_time_stamp() << " ";
            cout << "x" << rd << " = x" << rs1 << " * x" << rs2 << endl;
            #endif

            r.incrementPc();
            return cycleTime;
        }
        else if(cmd == lw)
        {
            int32_t word  = 0;
            uint64_t addr = r.get(rs1)+imm;

            do_b_transport(addr,
                           tlm::TLM_READ_COMMAND,
                           reinterpret_cast<unsigned char*>(&word),
                           delay);

            r.set(rd, word);

            #ifdef DEBUG
            cout << "@" << sc_time_stamp() << " LW:";
            cout << "x" << rd << " = 0x" << hex << addr
                 << dec << "("<< word << ")" << endl;
            #endif

            r.incrementPc();
            return delay;
        }
        else if(cmd == sw)
        {
            uint32_t im1 = (data & 0b11111110000000000000000000000000) >> 25;
            uint32_t im2 = (data & 0b00000000000000000000111110000000) >> 7;

            // Sign extension:
            bool sign = (data & 0b10000000000000000000000000000000) >> 31;

            if(sign == false)
            {
                imm = im2 | (im1 << 5);
            }
            else
            {
                imm = im2 | (im1 << 5) | 0b11111111111111111111000000000000;
            }

            int32_t word  = r.get(rs2);
            sc_time delay = SC_ZERO_TIME;
            uint64_t addr = r.get(rs1)+imm;

            do_b_transport(addr,
                           tlm::TLM_WRITE_COMMAND,
                           reinterpret_cast<unsigned char*>(&word),
                           delay);

            #ifdef DEBUG
            cout << "@" << sc_time_stamp() << " SW:";
            cout << "0x" << hex << addr << dec << " = " << word << endl;
            #endif

            r.incrementPc();
            return delay;
        }
        else if(cmd == jr)
        {
            #ifdef DEBUG
            cout << "@" << sc_time_stamp() << " ";
            cout << "jr " << r.get(rs1) << endl;
            #endif

            r.setPc(r.get(rs1));
            return cycleTime;
        }
        else if(cmd == jal)
        {
            imm = (data & 0b11111111111111111111000000000000) >> 12;

            bool sign = (data & 0b10000000000000000000000000000000) >> 31;

            if(sign == true)
            {
                imm = imm | 0b11111111111100000000000000000000;
            }

            #ifdef DEBUG
            cout << "@" << sc_time_stamp() << " ";
            cout << "jal " << r.get(rs1) << endl;
            #endif

            r.set(rd,r.getPc()+4);
            r.setPc(r.getPc()+imm);
            return cycleTime;
        }
        else if(cmd == bne)
        {
            uint32_t im1 = (data & 0b11111110000000000000000000000000) >> 25;
            uint32_t im2 = (data & 0b00000000000000000000111110000000) >> 7;

            // Sign extension:
            bool sign = (data & 0b10000000000000000000000000000000) >> 31;

            if(sign == false)
            {
                imm = im2 | (im1 << 5);
            }
            else
            {
                imm = im2 | (im1 << 5) | 0b11111111111111111111000000000000;
            }

            if(r.get(rs1) != r.get(rs2))
            {
                r.setPc(r.getPc() + imm);
            }
            else
            {
                r.incrementPc();
            }

            #ifdef DEBUG
            cout << "@" << sc_time_stamp() << " ";
            cout << "bne " << r.getPc() << endl;
            #endif

            return delay;
        }

    }

};

#endif // CPU_H
