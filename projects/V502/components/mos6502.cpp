#include "mos6502.hpp"

#include "memory.hpp"

#include <operations/operation.hpp>

namespace V502 {
    inline uint16_t make_wide(uint8_t a, uint8_t b) {
        return (uint16_t)a >> 8 | (uint16_t)b;
    }

    MOS6502::MOS6502() {
        program_memory = nullptr;
        system_memory = nullptr;
    }

    // Executes the next instruction and increments the program counter!
    // Returns false if the CPU has reached some dead end!
    bool MOS6502::cycle() {
        if (!program_memory)
            throw new std::runtime_error("6502 was missing program memory!");

        if (!system_memory)
            throw new std::runtime_error("6502 was missing system memory!");

        // TODO: Move instruction operations somewhere else
        // TODO: Unprototype this code
        uint8_t op = (*program_memory)[program_counter];

        uint8_t a = 0;
        uint8_t b = 0;

        bool increment = true;

        try {
            increment = OPERATIONS[op](op, this);
        } catch (std::exception& err) {

        }

        if (increment)
            program_counter += 1;

        // TODO: Failure states, such as getting to the end of the program with nothing more, or faults!
        return true;
    }

    // TODO: Nullptr check
    uint8_t MOS6502::next_program() {
        return program_memory->at(++program_counter);
    }

    uint16_t MOS6502::next_program_wide() {
        return make_wide(next_program(), next_program());
    }

    void MOS6502::store_at(uint16_t idx, uint8_t val) {
        system_memory->at(idx) = val;
    }

    // Used by X offset ZPG variants to help with wrapping around by purposefully overflowing an 8 bit int
    void MOS6502::store_at_page(uint8_t page, uint8_t idx, uint8_t val) {
        MOS6502::store_at(make_wide(page, idx), val);
    }

    void MOS6502::jump(uint16_t idx) {
        program_counter = idx;
    }
}