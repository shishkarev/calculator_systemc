#include "testbench.hpp"
#include <iostream>

Testbench::Testbench(sc_module_name name, Memory* mem) : sc_module(name), memory(mem) {
    SC_METHOD(initialize_program);
    sensitive << start_signal;
}

void Testbench::initialize_program() {
    Instruction program[] = {
        {InstructionType::LOAD, 0, 0, 0, 0x100},
        {InstructionType::LOAD, 1, 0, 0, 0x104},
        {InstructionType::ADD, 2, 0, 1, 0},
        {InstructionType::STORE, 2, 0, 0, 0x108},
        {InstructionType::HALT, 0, 0, 0, 0}
    };

    for (size_t i = 0; i < sizeof(program) / sizeof(program[0]); ++i) {
        memory->write(0x200 + i * sizeof(Instruction), reinterpret_cast<const uint8_t*>(&program[i]), sizeof(Instruction));
    }

    uint32_t data1 = 10;
    uint32_t data2 = 20;
    memory->write(0x100, data1);
    memory->write(0x104, data2);

    start_signal.write(true);
    pc_start_signal.write(0x200);
}

uint32_t Testbench::read_memory(uint32_t addr) {
    return memory->read(addr);
}