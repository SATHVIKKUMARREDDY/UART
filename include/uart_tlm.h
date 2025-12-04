#pragma once

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include <queue>

using namespace sc_core;
using namespace tlm;
using namespace tlm_utils;
using namespace std;

class UART_TLM : public sc_module {
public:
    simple_target_socket<UART_TLM> target_socket;

    SC_CTOR(UART_TLM) : target_socket("target_socket"), baud_rate(9600) {
        target_socket.register_b_transport(this, &UART_TLM::b_transport);
        SC_THREAD(transmit_thread);
        SC_THREAD(receive_thread);
    }

    ~UART_TLM() {
        cout << "UART_TLM module destroyed" << endl;
    }

    void transmit_thread();
    void receive_thread();

private:
    uint32_t baud_rate;
    queue<uint8_t> rx_buffer;
    queue<uint8_t> tx_buffer;
    sc_event rx_event;
    sc_event tx_event;
    
    void b_transport(tlm_generic_payload& trans, sc_time& delay);
};