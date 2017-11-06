#ifndef UTILS_H
#define UTILS_H
#include<systemc.h>
#include <sysc/communication/sc_fifo.h>

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
        std::cout << "Matthias Jung, Éder Zulian, Mohsin Ghaffar (2017)" << std::endl;

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
