#ifndef TESTBENCH_HPP
#define TESTBENCH_HPP

#include <systemc>
#include <tlm>
#include "memory.hpp"
#include "types.hpp"

class Testbench : public sc_module {
public:
    sc_out<bool> start_signal;
    sc_out<uint32_t> pc_start_signal;
    
    Testbench(sc_module_name name, Memory* mem);
    void initialize_program();
    uint32_t read_memory(uint32_t addr);

private:
    Memory* memory;
    SC_HAS_PROCESS(Testbench);
};

#endif // TESTBENCH_HPP