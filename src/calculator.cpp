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

    bool running = true;
    while (running) {
        decode();
        read();
        execute();
        writeback();

        if (decode_reg.instr.type == HALT) {
            running = false;
        }

        wait(10, SC_NS);
    }
}

void Calculator::decode() {
    if (!start.read()) {
        decode_reg.valid = false;
        return;
    }

    Instruction instr;
    tlm::tlm_generic_payload trans;
    sc_time delay = SC_ZERO_TIME;
    tlm::tlm_phase phase = tlm::BEGIN_REQ;

    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_address(pc);
    trans.set_data_ptr(reinterpret_cast<unsigned char*>(&instr));
    trans.set_data_length(sizeof(instr));

    tlm::tlm_sync_enum status = memory_socket->nb_transport_fw(trans, phase, delay);
    if (status == tlm::TLM_UPDATED) {
        wait(delay);
        phase = tlm::BEGIN_RESP;
        status = memory_socket->nb_transport_fw(trans, phase, delay);
    } else if (status == tlm::TLM_COMPLETED) {
        SC_REPORT_ERROR("Calculator", "Instruction fetch failed - transaction completed unexpectedly");
    }

    if (trans.is_response_error()) {
        SC_REPORT_ERROR("Calculator", "Instruction fetch error");
    }

    pc += sizeof(Instruction);
    decode_reg.instr = instr;
    decode_reg.valid = true;
}

void Calculator::read() {
    if (!decode_reg.valid) {
        read_reg.valid = false;
        return;
    }

    read_reg.operand1 = regs[decode_reg.instr.reg1];
    read_reg.operand2 = regs[decode_reg.instr.reg2];
    read_reg.instr = decode_reg.instr;
    read_reg.valid = true;
}

void Calculator::execute() {
    if (!read_reg.valid) {
        execute_reg.valid = false;
        return;
    }

    switch (read_reg.instr.type) {
        case LOAD: execute_load(read_reg.instr); break;
        case ADD: execute_add(read_reg.instr); break;
        case STORE: execute_store(read_reg.instr); break;
        case HALT: break;
        default:
            SC_REPORT_ERROR("Calculator", "Unknown instruction");
            return;
    }

    execute_reg.instr = read_reg.instr;
    execute_reg.valid = true;
}

void Calculator::writeback() {
    if (!execute_reg.valid) {
        writeback_reg.valid = false;
        return;
    }

    if (execute_reg.instr.type == ADD) {
        regs[execute_reg.instr.reg1] = execute_reg.result;
    }

    writeback_reg.instr = execute_reg.instr;
    writeback_reg.result = execute_reg.result;
    writeback_reg.valid = true;
}

void Calculator::execute_load(const Instruction& instr) {
    uint32_t data;
    tlm::tlm_generic_payload trans;
    sc_time delay = SC_ZERO_TIME;
    tlm::tlm_phase phase = tlm::BEGIN_REQ;

    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_address(instr.address);
    trans.set_data_ptr(reinterpret_cast<unsigned char*>(&data));
    trans.set_data_length(sizeof(data));

    tlm::tlm_sync_enum status = memory_socket->nb_transport_fw(trans, phase, delay);
    if (status == tlm::TLM_UPDATED) {
        wait(delay);
        phase = tlm::BEGIN_RESP;
        status = memory_socket->nb_transport_fw(trans, phase, delay);
    } else if (status == tlm::TLM_COMPLETED) {
        SC_REPORT_ERROR("Calculator", "Memory read failed - transaction completed unexpectedly");
    }

    if (trans.is_response_error()) {
        SC_REPORT_ERROR("Calculator", "Memory read error");
    }

    regs[instr.reg1] = data;
    cout << "LOAD: R" << (int)instr.reg1 << " = mem[0x" << hex << instr.address
        << "] = " << dec << regs[instr.reg1] << endl;
}

void Calculator::execute_add(const Instruction& instr) {
    regs[instr.reg1] = regs[instr.reg2] + regs[instr.reg3];
    execute_reg.result = regs[instr.reg1];
    cout << "ADD: R" << (int)instr.reg1 << " = R" << (int)instr.reg2
        << " + R" << (int)instr.reg3 << " = " << dec << regs[instr.reg1] << endl;
}

void Calculator::execute_store(const Instruction& instr) {
    tlm::tlm_generic_payload trans;
    sc_time delay = SC_ZERO_TIME;
    tlm::tlm_phase phase = tlm::BEGIN_REQ;

    trans.set_command(tlm::TLM_WRITE_COMMAND);
    trans.set_address(instr.address);
    trans.set_data_ptr(reinterpret_cast<unsigned char*>(&regs[instr.reg1]));
    trans.set_data_length(sizeof(regs[instr.reg1]));

    tlm::tlm_sync_enum status = memory_socket->nb_transport_fw(trans, phase, delay);
    if (status == tlm::TLM_UPDATED) {
        wait(delay);
        phase = tlm::BEGIN_RESP;
        status = memory_socket->nb_transport_fw(trans, phase, delay);
    } else if (status == tlm::TLM_COMPLETED) {
        SC_REPORT_ERROR("Calculator", "Memory write failed - transaction completed unexpectedly");
    }

    if (trans.is_response_error()) {
        SC_REPORT_ERROR("Calculator", "Memory write error");
    }

    cout << "STORE: mem[0x" << hex << instr.address << "] = R"
        << (int)instr.reg1 << " = " << dec << regs[instr.reg1] << endl;
}

void Calculator::invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) {
}

tlm::tlm_sync_enum Calculator::nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay) {
    if (phase == tlm::BEGIN_RESP) {
        phase = tlm::END_RESP;
        return tlm::TLM_UPDATED;
    }
    return tlm::TLM_COMPLETED;
}