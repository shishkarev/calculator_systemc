#ifndef TYPES_HPP
#define TYPES_HPP

#include <systemc.h>

enum InstructionType {
    LOAD = 0x01,
    ADD = 0x02,
    STORE = 0x03,
    HALT = 0xFF
};

struct Instruction {
    InstructionType type;
    uint8_t reg1, reg2, reg3;
    uint32_t address;
};

#endif // TYPES_HPP