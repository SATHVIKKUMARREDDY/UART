#include "uart_tlm.h"
#include <systemc>
#include <tlm>
#include <iostream>
#include <tlm_utils/simple_initiator_socket.h>

using namespace sc_core;
using namespace std;

SC_MODULE(Testbench) {
    tlm_utils::simple_initiator_socket<Testbench> initiator;
    UART_TLM* uart;

    SC_CTOR(Testbench) : initiator("initiator") {
        SC_THREAD(run);
    }

    void run() {
        // Write data to UART TX
        for (uint8_t i = 0; i < 5; ++i) {
            tlm_generic_payload trans;
            uint8_t data = 'A' + i;
            trans.set_command(TLM_WRITE_COMMAND);
            trans.set_address(0x00);
            trans.set_data_ptr(&data);
            trans.set_data_length(1);
            trans.set_response_status(TLM_INCOMPLETE_RESPONSE);
            sc_time delay = SC_ZERO_TIME;

            initiator->b_transport(trans, delay);

            if (trans.is_response_error())
                SC_REPORT_FATAL("Testbench", "Response error from b_transport");

            wait(sc_time(100, SC_NS));  // increased delay
        }

        wait(sc_time(1, SC_MS));  // wait for transmit thread to process

        // Read data from UART RX
        for (uint8_t i = 0; i < 5; ++i) {
            // wait until rx_count > 0
            while (true) {
                tlm_generic_payload stat_trans;
                uint32_t rx_count = 0;
                stat_trans.set_command(TLM_READ_COMMAND);
                stat_trans.set_address(0x08);           // RX count/status
                stat_trans.set_data_ptr(reinterpret_cast<uint8_t*>(&rx_count));
                stat_trans.set_data_length(4);
                stat_trans.set_response_status(TLM_INCOMPLETE_RESPONSE);
                sc_time delay = SC_ZERO_TIME;

                initiator->b_transport(stat_trans, delay);
                wait(delay);                            // honor target timing
                if (stat_trans.is_response_error())
                    SC_REPORT_FATAL("Testbench", "Status read error");

                if (rx_count > 0) break;
                wait(sc_time(50, SC_US));               // poll interval
            }

            // now read one byte from RX (same as before)
            tlm_generic_payload trans;
            uint8_t data = 0;
            trans.set_command(TLM_READ_COMMAND);
            trans.set_address(0x04);         // RX register
            trans.set_data_ptr(&data);
            trans.set_data_length(1);
            trans.set_response_status(TLM_INCOMPLETE_RESPONSE);
            sc_time delay = SC_ZERO_TIME;

            initiator->b_transport(trans, delay);
            wait(delay);                        // honor read latency

            cout << "CPU read RX: " << data << endl;
            wait(sc_time(100, SC_NS));
        }
    }
};

int sc_main(int argc, char* argv[])
{
    UART_TLM uart("UART1");
    Testbench tb("Testbench");
    
    tb.initiator.bind(uart.target_socket);
    
    sc_start(5, SC_MS);

    return 0;
}