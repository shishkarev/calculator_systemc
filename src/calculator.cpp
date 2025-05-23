#include "calculator.hpp"
#include <iostream>

using namespace std;

Calculator::Calculator(sc_module_name name) : sc_module(name) {
    SC_THREAD(run);
    sensitive << start.pos();
    memset(regs, 0, sizeof(regs));
    memory_socket.bind(*this);
}

void Calculator::run() {
    wait();

    pc = pc_start.read();
    cout << "Calculator started at PC = 0x" << hex << pc << endl;

    while (true) {
        Instruction instr = read_instruction(pc);
        pc += sizeof(Instruction);

        switch (instr.type) {
            case LOAD: execute_load(instr); break;
            case ADD: execute_add(instr); break;
            case STORE: execute_store(instr); break;
            case HALT: return;
            default:
                SC_REPORT_ERROR("Calculator", "Unknown instruction");
                return;
        }

        wait(10, SC_NS);
    }
}

Instruction Calculator::read_instruction(uint32_t addr) {
    Instruction instr;
    tlm::tlm_generic_payload trans;
    sc_time delay = SC_ZERO_TIME;

    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_address(addr);
    trans.set_data_ptr(reinterpret_cast<unsigned char*>(&instr));
    trans.set_data_length(sizeof(instr));

    memory_socket->b_transport(trans, delay);

    if (trans.is_response_error()) {
        SC_REPORT_ERROR("Calculator", "Instruction fetch error");
    }

    return instr;
}

uint32_t Calculator::read_memory(uint32_t addr) {
    uint32_t data;
    tlm::tlm_generic_payload trans;
    sc_time delay = SC_ZERO_TIME;

    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_address(addr);
    trans.set_data_ptr(reinterpret_cast<unsigned char*>(&data));
    trans.set_data_length(sizeof(data));

    memory_socket->b_transport(trans, delay);

    if (trans.is_response_error()) {
        SC_REPORT_ERROR("Calculator", "Memory read error");
    }

    return data;
}

void Calculator::write_memory(uint32_t addr, uint32_t value) {
    tlm::tlm_generic_payload trans;
    sc_time delay = SC_ZERO_TIME;

    trans.set_command(tlm::TLM_WRITE_COMMAND);
    trans.set_address(addr);
    trans.set_data_ptr(reinterpret_cast<unsigned char*>(&value));
    trans.set_data_length(sizeof(value));

    memory_socket->b_transport(trans, delay);

    if (trans.is_response_error()) {
        SC_REPORT_ERROR("Calculator", "Memory write error");
    }
}

void Calculator::execute_load(const Instruction& instr) {
    regs[instr.reg1] = read_memory(instr.address);
    cout << "LOAD: R" << (int)instr.reg1 << " = mem[0x" << hex << instr.address
        << "] = " << dec << regs[instr.reg1] << endl;
}

void Calculator::execute_add(const Instruction& instr) {
    regs[instr.reg1] = regs[instr.reg2] + regs[instr.reg3];
    cout << "ADD: R" << (int)instr.reg1 << " = R" << (int)instr.reg2
        << " + R" << (int)instr.reg3 << " = " << dec << regs[instr.reg1] << endl;
}

void Calculator::execute_store(const Instruction& instr) {
    write_memory(instr.address, regs[instr.reg1]);
    cout << "STORE: mem[0x" << hex << instr.address << "] = R"
        << (int)instr.reg1 << " = " << dec << regs[instr.reg1] << endl;
}

void Calculator::invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) {
}

tlm::tlm_sync_enum Calculator::nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay) {
    return tlm::TLM_COMPLETED;
}