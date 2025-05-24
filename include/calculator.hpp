#ifndef CALCULATOR_HPP
#define CALCULATOR_HPP

#include <systemc>
#include <tlm.h>
#include "types.hpp"

using namespace sc_core;

class Calculator : public sc_module, public tlm::tlm_bw_transport_if<> {
public:
    sc_in<bool> start;
    sc_in<bool> reset;
    sc_in<uint32_t> pc_start;
    tlm::tlm_initiator_socket<> memory_socket;

    SC_HAS_PROCESS(Calculator);
    Calculator(sc_module_name name);

    void run();

    virtual tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay) override;
    virtual void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) override;

private:
    struct Decode {
        Instruction instr;
        bool valid;
        Decode() : valid(false) {}
    };

    struct Read {
        Instruction instr;
        uint32_t operand1;
        uint32_t operand2;
        bool valid;
        Read() : valid(false) {}
    };

    struct Execute {
        Instruction instr;
        uint32_t result;
        bool valid;
        Execute() : valid(false) {}
    };

    struct Writeback {
        Instruction instr;
        uint32_t result;
        bool valid;
        Writeback() : valid(false) {}
    };

    Decode decode_reg;
    Read read_reg;
    Execute execute_reg;
    Writeback writeback_reg;

    uint32_t regs[4];
    uint32_t pc;

    void decode();
    void read();
    void execute();
    void writeback();

    void execute_load(const Instruction& instr);
    void execute_add(const Instruction& instr);
    void execute_store(const Instruction& instr);
};

#endif // CALCULATOR_HPP