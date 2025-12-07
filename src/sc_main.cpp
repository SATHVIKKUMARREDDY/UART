#include "uart_tlm.h"
#include "testbench.h"
#include <systemc>

// ------------------------ Main Simulation Entry Point ------------------------
int sc_main(int argc, char* argv[]) {
    // Create UART peripheral instance with baud rate 115200
    UART_TLM uart("UART1", 115200);

    // Create Testbench instance
    Testbench tb("TB");

    // Bind Testbench initiator socket to UART target socket
    tb.initiator.bind(uart.socket);

    // Start SystemC simulation for 50 milliseconds
    sc_core::sc_start(50, sc_core::SC_MS);

    // Return success
    return 0;
}
