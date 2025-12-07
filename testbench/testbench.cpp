#include "uart_tlm.h"
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <iostream>
#include "testbench.h"


// ------------------------ Main Testbench Thread ------------------------
void Testbench::run() {
    // ------------------------ Write data to UART TX ------------------------
    for (uint8_t i = 0; i < 5; ++i) {
        // Wait until TX_READY is set in the UART STATUS register
        while (true) {
            uint32_t status = 0;                         // variable to hold STATUS register value
            tlm::tlm_generic_payload stat_trans;         // create a TLM transaction
            stat_trans.set_command(tlm::TLM_READ_COMMAND);
            stat_trans.set_address(0x08);                // STATUS register address
            stat_trans.set_data_ptr(reinterpret_cast<uint8_t*>(&status));
            stat_trans.set_data_length(4);

            sc_core::sc_time delay = sc_core::sc_time(1, sc_core::SC_NS);   // small delay to let simulation advance
            initiator->b_transport(stat_trans, delay);  // read STATUS from UART
            sc_core::wait(delay);                        // wait for transaction to complete

            // Check for errors in transaction
            if (stat_trans.is_response_error())
                SC_REPORT_FATAL("Testbench", "Status read error");

            // Break loop if TX is ready
            if (status & (1 << 1))  // TX_READY is bit 1
                break;

            sc_core::wait(sc_core::sc_time(10, sc_core::SC_NS));  // poll interval to avoid busy-waiting
        }


        // ------------------------ Write one byte to TX register ------------------------
        tlm::tlm_generic_payload trans;
        uint8_t data = 'A' + i;       // write ASCII characters 'A', 'B', ...
        trans.set_command(tlm::TLM_WRITE_COMMAND);
        trans.set_address(0x00);      // TX register address
        trans.set_data_ptr(&data);
        trans.set_data_length(1);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

        sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
        initiator->b_transport(trans, delay);  // write the byte to UART
        sc_core::wait(delay);                  // wait for write to complete

        if (trans.is_response_error())
            SC_REPORT_FATAL("Testbench", "Response error from b_transport");

        std::cout << "CPU wrote TX: " << data << " at " << sc_core::sc_time_stamp() << std::endl;
    }

    // Wait a short time to allow the UART transmit thread to process bytes
    sc_core::wait(sc_core::sc_time(10, sc_core::SC_NS));

    // ------------------------ Read data from UART RX ------------------------
    for (uint8_t i = 0; i < 5; ++i) {
        // Wait until RX_COUNT > 0 (data available in RX buffer)
        while (true) {
            uint32_t rx_count = 0;
            tlm::tlm_generic_payload stat_trans;
            stat_trans.set_command(tlm::TLM_READ_COMMAND);
            stat_trans.set_address(0x08);         // STATUS register / RX count
            stat_trans.set_data_ptr(reinterpret_cast<uint8_t*>(&rx_count));
            stat_trans.set_data_length(4);

            sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
            initiator->b_transport(stat_trans, delay); // read STATUS / RX count
            sc_core::wait(delay);

            if (stat_trans.is_response_error())
                SC_REPORT_FATAL("Testbench", "Status read error");

            if (rx_count > 0) break;             // exit loop when data is available
            sc_core::wait(sc_core::sc_time(10, sc_core::SC_NS)); // poll interval
        }

        // Now read one byte from RX register
        tlm::tlm_generic_payload trans;
        uint8_t data = 0;
        trans.set_command(tlm::TLM_READ_COMMAND);
        trans.set_address(0x04);                 // RX register address
        trans.set_data_ptr(&data);
        trans.set_data_length(1);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

        sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
        initiator->b_transport(trans, delay);    // read the byte
        sc_core::wait(delay);

        std::cout << "CPU read RX: " << data << " at " << sc_core::sc_time_stamp() << std::endl;
    }
}
