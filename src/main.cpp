#include <systemc>
#include <memory>
#include "testbench.hpp"
#include "calculator.hpp"
#include "memory.hpp"

using namespace sc_core;

int sc_main(int argc, char* argv[]) {
    Memory memory("memory", 1024);
    Calculator calculator("calculator");
    Testbench testbench("testbench", &memory);

    sc_signal<bool> start_signal;
    sc_signal<uint32_t> pc_start_signal;
    sc_signal<bool> reset_signal;

    testbench.start_signal(start_signal);
    testbench.pc_start_signal(pc_start_signal);
    calculator.start(start_signal);
    calculator.pc_start(pc_start_signal);
    calculator.reset(reset_signal);
    calculator.memory_socket.bind(memory.socket);

    sc_start();
    return 0;
}