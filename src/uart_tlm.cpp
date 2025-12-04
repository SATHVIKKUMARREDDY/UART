#include "uart_tlm.h"
#include <iostream>
#include <cstring>

using namespace std;

// ------------------------ b_transport ------------------------
// Simple memory-mapped UART registers
// addr 0x00 -> TX register (write)
// addr 0x04 -> RX register (read)
// ------------------------ Transmit thread ------------------------
void UART_TLM::b_transport(tlm_generic_payload& trans, sc_time& delay)
{
    uint8_t* ptr = trans.get_data_ptr();
    uint64_t addr = trans.get_address();
    tlm_command cmd = trans.get_command();

    if (cmd == TLM_WRITE_COMMAND) {
        if (addr == 0x00) {       // TX register
            tx_buffer.push(*ptr);
            tx_event.notify();    // trigger transmit thread
            cout << "@" << sc_time_stamp() << " UART TX write: "
                 << (int)(*ptr) << endl;
        } else {
            trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }
    } else if (cmd == TLM_READ_COMMAND) {
        if (addr == 0x04) {       // RX register
            if (!rx_buffer.empty()) {
                *ptr = rx_buffer.front();
                rx_buffer.pop();
            } else {
                *ptr = 0;
            }
        } else {
            trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }
    }

    delay += sc_time(10, SC_NS);
    trans.set_response_status(TLM_OK_RESPONSE);
}

void UART_TLM::transmit_thread()
{
    while (true) {
        wait(tx_event);

        while (!tx_buffer.empty()) {
            uint8_t data = tx_buffer.front();
            tx_buffer.pop();
            
            // Simulate baud rate delay (1 bit time at 9600 baud)
            wait(sc_time(104, SC_US));  // ~10 bits per byte
            
            // Loopback: put data in RX buffer
            rx_buffer.push(data);
            rx_event.notify();
        }
    }
}
// ------------------------ Receive thread ------------------------
void UART_TLM::receive_thread()
{
    while (true) {
        wait(rx_event);

        // Just log RX data, don't consume it
        if (!rx_buffer.empty()) {
            cout << "@" << sc_time_stamp() << " UART received: "
                 << (int)rx_buffer.front() << endl;
        }
    }
}