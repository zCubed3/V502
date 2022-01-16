#ifndef V502_MOS6502_HPP
#define V502_MOS6502_HPP

#include <stdint.h>

namespace V502 {
    class Memory;

    // The 6502 is represented here by its name because we might add the W65C02 some day!
    // If we do that though a lot of stuff will have to shift around
    class MOS6502 {
    public:
        // The 6502 is laid out as close to its real counterpart as possible
        // We have a program counter, instruction decoder, etc...

        // TODO: Timing table and virtual CPU clock!

        // Some basic typedefs to make things stand out better

        typedef uint8_t register_t; // 8 bit register
        typedef uint16_t wregister_t; // 16 bit register

        //
        // Registers
        //
        wregister_t program_counter;

        register_t stack_ptr;
        register_t accumulator;
        register_t index_x;
        register_t index_y;
        register_t flags;

        //
        // CPU Flags
        //
        enum class Flags : register_t {
            Carry       = 0b00000001,
            Zero        = 0b00000010,
            Interrupt   = 0b00000100,
            Decimal     = 0b00001000,
            Break       = 0b00010000,
            // Bit 5 has no purpose
            Overflow    = 0b01000000,
            Negative    = 0b10000000
        };

        //
        // Memory
        //

        // Just like the real deal, it's a DIY sort of deal here! Provide your own program memory and system memory!
        // If you forget one the CPU will throw an exception!
        Memory *program_memory;
        Memory *system_memory;

        // Helpers for operations to reduce repeated code
        uint8_t next_program();
        uint16_t next_program_wide();
        void store_at(uint16_t idx, uint8_t val);
        void store_at_page(uint8_t page, uint8_t idx, uint8_t val);
        void jump(uint16_t idx);

        MOS6502();
        bool cycle();
    };
}

#endif
