# UART TLM SystemC Simulation

A Transaction Level Modeling (TLM) implementation of a UART (Universal Asynchronous Receiver/Transmitter) in SystemC.

## Overview

This project demonstrates TLM-based communication between a testbench and a UART module using SystemC. The UART simulates serial communication at 9600 baud with loopback functionality for testing.

## Features

- **TLM-2.0 Compliant**: Uses `simple_initiator_socket` and `simple_target_socket`
- **Memory-Mapped Registers**:
  - `0x00`: TX (Transmit) Register - Write only
  - `0x04`: RX (Receive) Register - Read only
- **Concurrent Threads**: Simulates transmit and receive operations
- **Baud Rate Simulation**: Realistic 104µs delay per byte at 9600 baud
- **Loopback Mode**: RX receives data from TX for testing

## Project Structure

```
uart_systemc/
├── CMakeLists.txt          # Build configuration
├── include/
│   └── uart_tlm.h          # UART module header
├── src/
│   ├── uart_tlm.cpp        # UART implementation
│   └── sc_main.cpp         # Testbench and simulation
├── build/                  # Build directory
└── README.md              # This file
```

## Requirements

- **SystemC 2.3.3** or later
- **CMake 3.10** or later
- **C++14** or later compiler (g++, clang++)
- **Linux/WSL** environment

## Installation

### 1. Install SystemC

```bash
# Download and install SystemC 2.3.3
cd /tmp
wget https://www.accellera.org/images/downloads/standards/systemc/systemc-2.3.3.tar.gz
tar xzf systemc-2.3.3.tar.gz
cd systemc-2.3.3
mkdir build && cd build
../configure --prefix=/opt/systemc
make -j$(nproc)
sudo make install
```

### 2. Set Environment Variable

```bash
export SYSTEMC_HOME=/opt/systemc
```

### 3. Build the Project

```bash
cd uart_systemc
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Running the Simulation

```bash
cd build
./uart_tlm
```

## Expected Output

```
SystemC 2.3.3-Accellera --- Dec  4 2025 13:54:12
        Copyright (c) 1996-2018 by all Contributors,
        ALL RIGHTS RESERVED
@0 s UART TX write: 65
@100 ns UART TX write: 66
@200 ns UART TX write: 67
@300 ns UART TX write: 68
@400 ns UART TX write: 69
@104 us UART received: 65
@208 us UART received: 66
@312 us UART received: 67
@416 us UART received: 68
@520 us UART received: 69
CPU read RX: A
CPU read RX: B
CPU read RX: C
CPU read RX: D
CPU read RX: E
UART_TLM module destroyed
```

## How It Works

### Transaction Flow

1. **Testbench** initiates a write transaction to TX register (0x00)
2. **UART b_transport()** receives the transaction and queues data in TX buffer
3. **Transmit Thread** waits for TX data, simulates 104µs baud delay, pushes to RX buffer
4. **Testbench** reads from RX register (0x04)
5. **UART b_transport()** returns data from RX buffer

### Memory-Mapped I/O

| Address | Operation | Purpose |
|---------|-----------|---------|
| 0x00    | Write     | TX Register (Transmit) |
| 0x04    | Read      | RX Register (Receive) |

### Key Components

- **`tlm_generic_payload`**: Carries transaction data (command, address, data)
- **`simple_initiator_socket`**: Testbench sends transactions
- **`simple_target_socket`**: UART receives transactions
- **`SC_THREAD`**: Concurrent simulation of TX/RX operations
- **`sc_event`**: Synchronization between threads

## Code Example

```cpp
// Writing to UART TX
tlm_generic_payload trans;
uint8_t data = 'A';
trans.set_command(TLM_WRITE_COMMAND);
trans.set_address(0x00);  // TX register
trans.set_data_ptr(&data);
trans.set_data_length(1);
initiator->b_transport(trans, delay);

// Reading from UART RX
trans.set_command(TLM_READ_COMMAND);
trans.set_address(0x04);  // RX register
initiator->b_transport(trans, delay);
cout << "Received: " << data << endl;
```

## Future Enhancements

- [ ] Add FIFO status registers (empty/full flags)
- [ ] Implement interrupt signals
- [ ] Configurable baud rates
- [ ] Parity and stop bit handling
- [ ] Real serial port integration

## References

- [SystemC Documentation](https://www.accellera.org/images/downloads/standards/systemc/systemc_language_reference_manual.pdf)
- [TLM-2.0 Standard](https://www.accellera.org/downloads/standards/tlm/)

## License

This project is provided as-is for educational purposes.

## Author

Sathvik Kumar Reddy
