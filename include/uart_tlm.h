#ifndef UART_TLM_H
#define UART_TLM_H

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <queue>
#include <cstdint>
#include <cstring>

// ------------------------ UART_TLM Module ------------------------
// This class models a UART peripheral using SystemC TLM.
// It supports TX/RX buffers, status register, and simple timing based on baud rate.
class UART_TLM : public sc_core::sc_module
{
public:
    // TLM target socket to receive read/write transactions
    tlm_utils::simple_target_socket<UART_TLM> socket;

    // ------------------------ Constructors ------------------------
    SC_CTOR(UART_TLM);                         // Default constructor (uses default baud rate)
    UART_TLM(sc_core::sc_module_name name, double baud); // Constructor with custom baud rate

    // ------------------------ TLM b_transport ------------------------
    void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);

    // ------------------------ Threads ------------------------
    void transmit_thread();       // Simulates UART transmission (TX)
    void receive_event_thread();  // Handles received data (RX)

    // ------------------------ Status Register ------------------------
    uint32_t status_reg() const;  // Returns TX/RX status as a 32-bit register

private:
    // ------------------------ Buffers ------------------------
    std::queue<uint8_t> tx_buffer;  // Transmit buffer
    std::queue<uint8_t> rx_buffer;  // Receive buffer

    // ------------------------ Flags ------------------------
    bool tx_busy;   // UART is currently transmitting
    bool tx_ready;  // UART is ready to transmit
    bool rx_ready;  // Data available in RX buffer

    // ------------------------ Timing ------------------------
    double baud_rate;           // UART baud rate in bits per second
    sc_core::sc_time bit_time;  // Time for one bit
    sc_core::sc_time byte_time; // Time for one byte (start + 8 data + stop)

    // ------------------------ Events ------------------------
    sc_core::sc_event tx_event;       // Notifies transmit thread to start sending
    sc_core::sc_event tx_done_event;  // Notifies TX completion (optional)
    sc_core::sc_event rx_event;       // Notifies data received in RX buffer
};

#endif
