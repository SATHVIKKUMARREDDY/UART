#ifndef TB_UART_H
#define TB_UART_H

#include "uart_tlm.h"
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <iostream>

// ------------------------ Testbench Module ------------------------
// A simple testbench that writes data to and reads data from a UART_TLM peripheral
SC_MODULE(Testbench) {
    // TLM initiator socket to send transactions to UART
    tlm_utils::simple_initiator_socket<Testbench> initiator;

    SC_CTOR(Testbench) {
        SC_THREAD(run);
    }
    // ------------------------ Main Testbench Thread ------------------------
    void run();  // Implementation in tb_uart.cpp or inlined
};

#endif // TB_UART_H
