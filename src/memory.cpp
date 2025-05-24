#include "memory.hpp"

Memory::Memory(sc_module_name name, uint32_t size) : sc_module(name), data(size) {
    socket.bind(*this);
}

void Memory::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
    if (trans.get_address() >= data.size()) {
        trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        return;
    }

    if (trans.get_command() == tlm::TLM_READ_COMMAND) {
        memcpy(trans.get_data_ptr(), &data[trans.get_address()], trans.get_data_length());
    } else if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {
        memcpy(&data[trans.get_address()], trans.get_data_ptr(), trans.get_data_length());
    } else {
        trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
        return;
    }

    trans.set_response_status(tlm::TLM_OK_RESPONSE);
    delay += sc_time(10, SC_NS);
}

bool Memory::get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data) {
    return false;
}

tlm::tlm_sync_enum Memory::nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay) {
    if (phase == tlm::BEGIN_REQ) {
        if (trans.get_address() >= data.size()) {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            phase = tlm::END_REQ;
            return tlm::TLM_COMPLETED;
        }

        if (trans.get_command() == tlm::TLM_READ_COMMAND) {
            memcpy(trans.get_data_ptr(), &data[trans.get_address()], trans.get_data_length());
        } else if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {
            memcpy(&data[trans.get_address()], trans.get_data_ptr(), trans.get_data_length());
        } else {
            trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
            phase = tlm::END_REQ;
            return tlm::TLM_COMPLETED;
        }

        trans.set_response_status(tlm::TLM_OK_RESPONSE);
        phase = tlm::END_REQ;
        delay += sc_time(10, SC_NS);
        return tlm::TLM_UPDATED;
    } else if (phase == tlm::BEGIN_RESP) {
        phase = tlm::END_RESP;
        return tlm::TLM_UPDATED;
    }
    return tlm::TLM_COMPLETED;
}

unsigned int Memory::transport_dbg(tlm::tlm_generic_payload& trans) {
    return 0;
}

void Memory::write(uint32_t addr, const uint8_t* buf, uint32_t size) {
    if (addr + size > data.size()) {
        throw std::out_of_range("Memory write: address " + std::to_string(addr) + 
                              " with size " + std::to_string(size) + 
                              " exceeds memory size " + std::to_string(data.size()));
    }
    memcpy(&data[addr], buf, size);
}

void Memory::write(uint32_t addr, uint32_t value) {
    if (addr + sizeof(value) > data.size()) {
        throw std::out_of_range("Memory write: address " + std::to_string(addr) + 
                              " with size " + std::to_string(sizeof(value)) + 
                              " exceeds memory size " + std::to_string(data.size()));
    }
    memcpy(&data[addr], &value, sizeof(value));
}

uint32_t Memory::read(uint32_t addr) {
    if (addr + sizeof(uint32_t) > data.size()) {
        throw std::out_of_range("Memory read: address " + std::to_string(addr) + 
                              " with size " + std::to_string(sizeof(uint32_t)) + 
                              " exceeds memory size " + std::to_string(data.size()));
    }
    uint32_t value;
    memcpy(&value, &data[addr], sizeof(value));
    return value;
}