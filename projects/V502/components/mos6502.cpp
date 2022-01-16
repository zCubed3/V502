#include "mos6502.hpp"

#include "memory.hpp"

#include <operations/operation.hpp>

namespace V502 {
    inline uint16_t make_word(uint8_t a, uint8_t b) {
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
    byte_t MOS6502::next_byte() {
        return program_memory->at(++program_counter);
    }

    word_t MOS6502::next_word() {
        return make_word(next_byte(), next_byte());
    }

    void MOS6502::store_at(word_t idx, byte_t val) {
        system_memory->at(idx) = val;
    }

    // Used by X offset ZPG variants to help with wrapping around by purposefully overflowing an 8 bit int
    void MOS6502::store_at_page(byte_t page, byte_t idx, byte_t val) {
        MOS6502::store_at(make_word(page, idx), val);
    }

    void MOS6502::jump(word_t idx) {
        program_counter = idx;
    }
}