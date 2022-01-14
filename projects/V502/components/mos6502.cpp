#include "mos6502.hpp"

#include "memory.hpp"

namespace V502 {
    inline uint16_t make_pair(uint8_t a, uint8_t b) {
        return (uint16_t)a >> 8 | (uint16_t)b;
    }

    MOS6502::MOS6502() {
        program_memory = nullptr;
        system_memory = nullptr;
    }

    // Executes the next instruction and increments the program counter!
    // Returns false if the CPU has reached some dead end!
    bool MOS6502::Cycle() {
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
        switch ((OpCode)op) {
            case OpCode::ADC_NOW:
                accumulator += program_memory->at(++program_counter);
                break;

            case OpCode::STA_ZPG:
                system_memory->at(program_memory->at(++program_counter)) = accumulator;
                break;

            case OpCode::JMP_ABS:
                program_counter = make_pair(program_memory->at(++program_counter), program_memory->at(++program_counter));
                increment = false;
                break;
        }

        if (increment)
            program_counter += 1;

        // TODO: Failure states, such as getting to the end of the program with nothing more, or faults!
        return true;
    }
}