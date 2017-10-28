#ifndef KPN_H
#define KPN_H

#include <systemc.h>


SC_MODULE(kpn)
{
  private:
    sc_fifo<double> a, b, y;
    void kpn_add();
    void kpn_a();
    void kpn_b();
    void kpn_y();
    void debug_thread();
    void print_fifo(int max, int value, std::string name);

  public:
    SC_CTOR(kpn) : a(10), b(10), y(20)
    {
       SC_THREAD(kpn_add);
       SC_THREAD(kpn_a);
       SC_THREAD(kpn_b);
       SC_THREAD(kpn_y);
       SC_THREAD(debug_thread);
       sensitive << a.data_read_event() << a.data_written_event()
                 << b.data_read_event() << b.data_written_event()
                 << y.data_read_event() << y.data_written_event();
    }
};

#endif // KPN_H
