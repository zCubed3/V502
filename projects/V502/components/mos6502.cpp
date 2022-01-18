#include "mos6502.hpp"

#include "memory.hpp"

#include <operations/operation.hpp>

namespace V502 {
    inline uint16_t make_word(uint8_t a, uint8_t b) {
        return (a << 8) | b;
    }

    MOS6502::MOS6502() {
        system_memory = nullptr;
    }

    // Executes the next instruction and increments the program counter!
    // Returns false if the CPU has reached some dead end!
    bool MOS6502::cycle() {
        if (!system_memory)
            throw new std::runtime_error("6502 was missing system memory!");

        bool increment = true;

        byte_t op = system_memory->at(program_counter);
        increment = OPERATIONS[op](op, this);

        if (increment)
            program_counter += 1;

        if (program_counter >= system_memory->size())
            return false;

        return true;
    }

    byte_t MOS6502::next_byte() {
        if (!system_memory)
            throw std::runtime_error("System memory is a nullptr!");

        return system_memory->at(++program_counter);
    }

    word_t MOS6502::next_word() {
        if (!system_memory)
            throw std::runtime_error("System memory is a nullptr!");

        word_t word = make_word(system_memory->at(program_counter + 2), system_memory->at(program_counter + 1));
        program_counter += 2;
        return word;
    }

    void MOS6502::store_at(word_t idx, byte_t val) {
        if (!system_memory)
            throw std::runtime_error("System memory is a nullptr!");

        system_memory->at(idx) = val;
    }

    // Used by X/Y offset ZPG variants to help with wrapping around by purposefully overflowing an 8 bit int
    void MOS6502::store_at_page(byte_t page, byte_t idx, byte_t val) {
        store_at(make_word(page, idx), val);
    }

    void MOS6502::jump(word_t idx) {
        program_counter = idx;
    }

    // CMP, CPX, and CPY
    void MOS6502::compare(byte_t lhs, byte_t rhs) {
        flags &= ~(Flags::Carry | Flags::Zero);

        if (lhs > rhs)
            flags |= (Flags::Carry);
        else if (lhs == rhs) {
            flags |= (Flags::Carry | Flags::Zero);
            flags &= ~Flags::Negative;
        }
    }

    // Checks if there was an overflow and sets the flag accordingly
    // Since ADC and SBC use this, we set carry equal to overflow
    void MOS6502::add_with_overflow(byte_t lhs, byte_t rhs, bool subtracting) {
        word_t r = lhs + (subtracting ? -(int8_t)rhs : rhs);

        if (r < 0x00 || r > 0xFF)
            flags |= (Flags::Overflow | Flags::Carry);
        else {
            flags &= ~(Flags::Overflow | Flags::Carry);
        }

        accumulator = lhs + (subtracting ? -(int8_t)rhs : rhs);
    }

    // Resets the program counter and flags
    void MOS6502::reset() {
        if (!system_memory)
            throw std::runtime_error("System memory is a nullptr!");

        flags = 0;
        accumulator = 0;
        index_x = 0;
        index_y = 0;
        stack_ptr = 0xFF;
        program_counter = make_word(system_memory->at(0xFFFD), system_memory->at(0xFFFC));
    }

    byte_t MOS6502::get_at(word_t idx) {
        if (!system_memory)
            throw std::runtime_error("System memory is a nullptr!");

        return system_memory->at(idx);
    }

    byte_t MOS6502::get_at_page(byte_t page, byte_t idx) {
        if (!system_memory)
            throw std::runtime_error("System memory is a nullptr!");

        return system_memory->at(make_word(page, idx));
    }

    void MOS6502::jump_page(byte_t page, byte_t idx) {
        program_counter = make_word(page, idx);
    }

    void MOS6502::load(RegisterIndex reg, byte_t val) {
        byte_t& target = accumulator;
        switch (reg) {
            case RegisterIndex::X:
                target = index_x;
                break;

            case RegisterIndex::Y:
                target = index_y;
                break;
        }
        target = val;
    }

    byte_t MOS6502::get_indirect(byte_t page, byte_t idx, byte_t post_fetch) {
        word_t word = make_word(get_at_page(page, idx), get_at_page(page, idx + 1)) + post_fetch;
        return get_at(word);
    }
}