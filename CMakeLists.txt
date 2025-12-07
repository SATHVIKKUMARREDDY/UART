cmake_minimum_required(VERSION 3.10)
project(uart_tlm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Include SystemC headers
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    $ENV{SYSTEMC_HOME}/include
)

# Add executable
add_executable(uart_tlm
    src/uart_tlm.cpp
    src/sc_main.cpp
    testbench/testbench.cpp
)

# Link SystemC library explicitly
target_link_libraries(uart_tlm
    PRIVATE $ENV{SYSTEMC_HOME}/lib-linux64/libsystemc.a
)
