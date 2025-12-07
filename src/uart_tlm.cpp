#include "uart_tlm.h"
#include "testbench.h"
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <iostream>
#include <algorithm>
#include <cstring>

// ------------------------ Constructor (default baud rate) ------------------------
UART_TLM::UART_TLM(sc_core::sc_module_name name)
    : sc_core::sc_module(name),
      socket("socket"),
      tx_busy(false),
      tx_ready(true),
      rx_ready(false),
      baud_rate(115200)  // default baud rate
{
    // Compute timing for UART based on baud rate
    bit_time  = sc_core::sc_time(1.0 / baud_rate, sc_core::SC_SEC);
    byte_time = bit_time * 10; // 1 start + 8 data + 1 stop bit

    // Register TLM transport function
    socket.register_b_transport(this, &UART_TLM::b_transport);

    // Start transmit and receive threads
    SC_THREAD(transmit_thread);
    SC_THREAD(receive_event_thread);
}

// ------------------------ Constructor (custom baud rate) ------------------------
UART_TLM::UART_TLM(sc_core::sc_module_name name, double baud)
    : sc_core::sc_module(name),
      socket("socket"),
      tx_busy(false),
      tx_ready(true),
      rx_ready(false),
      baud_rate(baud)
{
    bit_time  = sc_core::sc_time(1.0 / baud_rate, sc_core::SC_SEC);
    byte_time = bit_time * 10;

    socket.register_b_transport(this, &UART_TLM::b_transport);

    SC_THREAD(transmit_thread);
    SC_THREAD(receive_event_thread);
}

// ------------------------ TLM b_transport function ------------------------
void UART_TLM::b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay)
{
    uint8_t* ptr = trans.get_data_ptr();
    uint64_t addr = trans.get_address();
    tlm::tlm_command cmd = trans.get_command();

    const sc_core::sc_time reg_access_time = sc_core::sc_time(10, sc_core::SC_NS);

    // -------- Write transaction --------
    if (cmd == tlm::TLM_WRITE_COMMAND) {
        if (addr == 0x00) {  // TX register
            if (!tx_ready) {
                trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
                return;
            }

            // Push byte to TX buffer and trigger transmit thread
            tx_buffer.push(*ptr);
            tx_event.notify();

            std::cout << "@" << sc_core::sc_time_stamp() << " UART TX write: " << (int)(*ptr) << std::endl;

            delay += reg_access_time;
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
            return;
        } else {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }
    }
    // -------- Read transaction --------
    else if (cmd == tlm::TLM_READ_COMMAND) {
        if (addr == 0x04) {  // RX register
            if (!rx_buffer.empty()) {
                *ptr = rx_buffer.front();
                rx_buffer.pop();
            } else {
                *ptr = 0;
            }
            delay += reg_access_time;
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
            return;
        } else if (addr == 0x08) { // STATUS / RX count
            uint32_t st = status_reg();  // get TX_READY, RX_READY, TX_BUSY
            size_t copy_len = std::min<size_t>(trans.get_data_length(), sizeof(st));
            std::memcpy(ptr, &st, copy_len);
            delay += reg_access_time;
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
            return;
        } else {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }
    }

    // Default error response if command not recognized
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
}

// ------------------------ Transmit Thread ------------------------
void UART_TLM::transmit_thread()
{
    while (true) {
        // Wait until TX event is triggered (data in TX buffer)
        sc_core::wait(tx_event);

        while (!tx_buffer.empty()) {
            uint8_t data = tx_buffer.front();
            tx_buffer.pop();

            tx_busy = true;
            tx_ready = false;

            // Simulate transmission time based on baud rate
            sc_core::wait(byte_time);

            // Loopback: push transmitted byte into RX buffer
            rx_buffer.push(data);
            rx_event.notify();

            std::cout << "@" << sc_core::sc_time_stamp() << " UART RX(loopback): " << (int)data << std::endl;

            tx_busy = false;
            tx_ready = true;
        }
    }
}

// ------------------------ Receive Event Thread ------------------------
void UART_TLM::receive_event_thread()
{
    while (true) {
        // Wait until RX event is triggered (data available)
        sc_core::wait(rx_event);

        if (!rx_buffer.empty()) {
            uint8_t data = rx_buffer.front();
            std::cout << "@" << sc_core::sc_time_stamp() << " UART RX ready: " << (int)data << std::endl;
        }
    }
}

// ------------------------ Status Register ------------------------
uint32_t UART_TLM::status_reg() const
{
    uint32_t st = 0;
    if (!rx_buffer.empty()) st |= 1 << 0;  // RX_READY
    if (tx_ready)            st |= 1 << 1;  // TX_READY
    if (tx_busy)             st |= 1 << 2;  // TX_BUSY
    return st;
}
