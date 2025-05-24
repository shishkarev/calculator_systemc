#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <systemc>
#include <tlm.h>
#include <vector>
#include <stdexcept>
#include <string>
#include "types.hpp"

using namespace sc_core;

class Memory : public sc_module, public tlm::tlm_fw_transport_if<> {
public:
    tlm::tlm_target_socket<> socket;

    SC_HAS_PROCESS(Memory);
    Memory(sc_module_name name, uint32_t size);

    virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) override;
    virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data) override;
    virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay) override;
    virtual unsigned int transport_dbg(tlm::tlm_generic_payload& trans) override;

    void write(uint32_t addr, const uint8_t* buf, uint32_t size);
    void write(uint32_t addr, uint32_t value);
    uint32_t read(uint32_t addr);

private:
    std::vector<uint8_t> data;
};

#endif // MEMORY_HPP