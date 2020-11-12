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
 *     - Éder F. Zulian
 */


#ifndef UTILS_H
#define UTILS_H

#include <systemc.h>
#include <sysc/communication/sc_fifo.h>
#include <unistd.h>

template <class T>
class my_sc_fifo : public sc_fifo<T>
{
  public:
    my_sc_fifo(const char* name_, int size_ = 16)
        : sc_fifo<T>(name_, size_)
    {
        fifos.push_back(this);
    }

  private:

    static std::vector< my_sc_fifo<T>* > fifos;

    int get_size()
    {
        return this->m_size;
    }

    int get_number()
    {
        int n = 0;
        if( this->m_free != this->m_size ) {
            int i = this->m_ri;
            do {
                i = ( i + 1 ) % this->m_size;
                n++;
            } while( i != this->m_wi );
        }
        return n;
    }

  public:
    void print_fifo()
    {
        int max = get_size();
        int n = get_number();

        std::cout << this->name() << " (" << max << ") " << "[";
        for(int i = 0; i < n; i++) {
            std::cout << "█";
        }
        for(int i = 0; i < max-n; i++) {
            std::cout << " ";
        }
        std::cout << "]" << std::endl;
        std::cout.flush();
    }

    static void print_fifos()
    {
        system("clear");
        std::cout << std::endl << "Kahn Process Network with SystemC" << std::endl;
        std::cout << "Matthias Jung, Éder Zulian (2017)" << std::endl;

        std::cout << std::endl << "δ: " << sc_delta_count() << std::endl;

        // Print all fifos:
        for(auto f : fifos)
        {
            f->print_fifo();
        }

        usleep(50000);
        //system("read");
    }

    T read()
    {
        T tmp = sc_fifo<T>::read();
        print_fifos();
        return tmp;
    }

    void write(const T& value)
    {
        sc_fifo<T>::write(value);
        print_fifos();
    }
};

template <class T>
std::vector< my_sc_fifo<T>* > my_sc_fifo<T>::fifos({});

#endif // UTILS_H
