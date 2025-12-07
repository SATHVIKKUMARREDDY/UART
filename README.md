# UART TLM SystemC Simulation

## Overview
This project simulates a UART (Universal Asynchronous Receiver/Transmitter) using **SystemC TLM-2.0**.  
It contains:  
- UART_TLM module with TX and RX (loopback)  
- Testbench to send and receive data  
- sc_main.cpp to connect and run the simulation  

## Project Structure

uart_systemc/
├── include/
│ ├── uart_tlm.h
│ └── testbench.h
├── src/
│ ├── uart_tlm.cpp
│ ├── sc_main.cpp
│ └── testbench.cpp
├── CMakeLists.txt
└── README.md


Copy code

## How to Build

```bash
mkdir build
cd build
cmake ..
make
./uart_tlm
Features
UART TX and RX simulation

Loopback of transmitted data

Polling for TX_READY and RX_READY

Configurable baud rate

TLM-2.0 b_transport for register access

Notes
Student project for learning SystemC TLM

Timing simulated based on baud rate

Currently supports only loopback; can be extended for multiple UARTs