#ifndef V502_MOS6502_HPP
#define V502_MOS6502_HPP

#include <stdint.h>

#include <v502common.hpp>

namespace V502 {
    class Memory;

    // The 6502 is represented here by its name because we might add the W65C02 some day!
    // While that CPU isn't that much different it would still be useful to have!
    class MOS6502 {
    public:
        // TODO: Timing table and virtual CPU clock!

        //
        // Registers
        //
        word_t program_counter;

        byte_t stack_ptr = 0xFF;
        byte_t accumulator;
        byte_t index_x;
        byte_t index_y;
        byte_t flags;

        //
        // CPU Flags
        //
        enum Flags : byte_t {
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
        byte_t next_byte();
        word_t next_word();
        void store_at(word_t idx, byte_t val);
        void store_at_page(byte_t page, byte_t idx, byte_t val);
        void jump(word_t idx);
        void compare(byte_t lhs, byte_t rhs);
        void add_with_overflow(byte_t lhs, byte_t rhs, bool subtracting = false);

        MOS6502();
        bool cycle();
    };
}

#endif
