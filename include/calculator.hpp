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
    uint32_t regs[4];
    uint32_t pc;

    Instruction read_instruction(uint32_t addr);
    uint32_t read_memory(uint32_t addr);
    void write_memory(uint32_t addr, uint32_t value);

    void execute_load(const Instruction& instr);
    void execute_add(const Instruction& instr);
    void execute_store(const Instruction& instr);
};

#endif //CALCULATOR_HPP