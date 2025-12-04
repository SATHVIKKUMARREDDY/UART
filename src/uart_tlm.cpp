#include "uart_tlm.h"
#include <iostream>
#include <cstring>

using namespace std;

// ------------------------ b_transport ------------------------
// Simple memory-mapped UART registers
// addr 0x00 -> TX register (write)
// addr 0x04 -> RX register (read)
// ------------------------ Transmit thread ------------------------
// ...existing code...
void UART_TLM::b_transport(tlm_generic_payload& trans, sc_time& delay)
{
    uint8_t* ptr = trans.get_data_ptr();
    uint64_t addr = trans.get_address();
    tlm_command cmd = trans.get_command();

    const sc_time reg_access_time = sc_time(10, SC_NS);

    if (cmd == TLM_WRITE_COMMAND) {
        if (addr == 0x00) {       // TX register
            tx_buffer.push(*ptr);
            tx_event.notify();    // trigger transmit thread
            cout << "@" << sc_time_stamp() << " UART TX write: "
                 << (int)(*ptr) << endl;
            delay += reg_access_time;
            trans.set_response_status(TLM_OK_RESPONSE);
            return;
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
            delay += reg_access_time;
            trans.set_response_status(TLM_OK_RESPONSE);
            return;
        } else if (addr == 0x08) { // RX count/status register (4 bytes)
            uint32_t cnt = static_cast<uint32_t>(rx_buffer.size());
            // copy up to requested length
            size_t copy_len = std::min<size_t>(trans.get_data_length(), sizeof(cnt));
            std::memcpy(ptr, &cnt, copy_len);
            delay += reg_access_time;
            trans.set_response_status(TLM_OK_RESPONSE);
            return;
        } else {
            trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }
    }

    trans.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
}

void UART_TLM::transmit_thread()
{
    // compute bit and byte time from baud_rate
    sc_time bit_time = sc_time(1.0 / static_cast<double>(baud_rate), SC_SEC);
    const unsigned bits_per_frame = 10; // start + 8 data + stop
    sc_time byte_time = bit_time * bits_per_frame;

    while (true) {
        wait(tx_event);

        while (!tx_buffer.empty()) {
            uint8_t data = tx_buffer.front();
            tx_buffer.pop();

            // simulate full byte transmission time
            wait(byte_time);

            // Loopback: put data in RX buffer and notify
            rx_buffer.push(data);
            rx_event.notify();

            // log here (one log per byte)
            cout << "@" << sc_time_stamp() << " UART received: " << (int)data << endl;
        }
    }
}

void UART_TLM::receive_thread()
{
    while (true) {
        wait(rx_event);

        // If you still want a logging thread, consume after logging
        if (!rx_buffer.empty()) {
            uint8_t v = rx_buffer.front();
            // optional: log again or perform other processing
            // cout << "@" << sc_time_stamp() << " RX-log: " << (int)v << endl;
            // do NOT pop here if the CPU/read should consume the byte
            // OR pop here if this thread is responsible for consuming RX
        }
    }
}