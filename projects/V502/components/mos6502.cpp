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

        bool increment = true;

        byte_t op = program_memory->at(program_counter);
        increment = OPERATIONS[op](op, this);

        if (increment)
            program_counter += 1;

        // TODO: Faults?
        if (program_counter >= program_memory->size())
            return false;

        return true;
    }

    byte_t MOS6502::next_byte() {
        if (!program_memory)
            throw std::runtime_error("Program memory is a nullptr!");

        return program_memory->at(++program_counter);
    }

    word_t MOS6502::next_word() {
        return make_word(next_byte(), next_byte());
    }

    void MOS6502::store_at(word_t idx, byte_t val) {
        if (!system_memory)
            throw std::runtime_error("System memory is a nullptr!");

        system_memory->at(idx) = val;
    }

    // Used by X/Y offset ZPG variants to help with wrapping around by purposefully overflowing an 8 bit int
    void MOS6502::store_at_page(byte_t page, byte_t idx, byte_t val) {
        MOS6502::store_at(make_word(page, idx), val);
    }

    void MOS6502::jump(word_t idx) {
        program_counter = idx;
    }

    // CMP, CPX, and CPY
    void MOS6502::compare(byte_t lhs, byte_t rhs) {
        // TODO: More compact?
        if (lhs > rhs)
            flags |= Flags::Carry;
        else
            flags &= ~Flags::Carry;

        if (lhs == rhs)
            flags |= Flags::Zero;
        else
            flags &= ~Flags::Zero;
    }
}