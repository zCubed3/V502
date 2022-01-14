#include "mos6502.hpp"

#include "memory.hpp"

namespace V502 {
    MOS6502::MOS6502() {
        program_memory = nullptr;
        system_memory = nullptr;
    }

    // Executes the next instruction and increments the program counter!
    void MOS6502::Cycle() {
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
                a = program_memory->at(++program_counter);
                b = program_memory->at(++program_counter);
                program_counter = (uint16_t)a >> 8 | (uint16_t)b;
                increment = false;
                break;
        }

        if (increment)
            program_counter += 1;
    }
}