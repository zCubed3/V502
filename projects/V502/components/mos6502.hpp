#ifndef V502_MOS6502_HPP
#define V502_MOS6502_HPP

#include <stdint.h>

#include <v502common.hpp>

namespace V502 {
    class Memory;

    // The 6502 is represented here by its name because we might add the W65C02 some day!
    // While that CPU isn't that much different it would still be useful to have!
    // TODO: W65C02S has more instructions than the 6502, we need a good way to make a 6502-like CPU abstract
    class MOS6502 {
    public:
        //
        // Registers
        //
        word_t program_counter = 0x0000;

        byte_t stack_ptr    = 0xFF;
        byte_t accumulator  = 0x00;
        byte_t index_x      = 0x00;
        byte_t index_y      = 0x00;
        byte_t flags        = 0x00;

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

        enum RegisterIndex : byte_t {
            A, X, Y
        };

        // Just like the real deal, it's a DIY sort of deal here! Provide your own memory!
        // If you forget this the CPU will throw an exception!
        Memory *system_memory;

        // Helpers for operations to consolidate behavior
        byte_t next_byte();
        word_t next_word();
        void store_at(word_t idx, byte_t val);
        void store_at_page(byte_t page, byte_t idx, byte_t val);
        void jump(word_t idx);
        void compare(byte_t lhs, byte_t rhs);
        void add_with_overflow(byte_t lhs, byte_t rhs, bool subtracting = false);
        void reset();
        byte_t get_at(word_t idx);
        byte_t get_at_page(byte_t page, byte_t idx);
        void jump_page(byte_t page, byte_t idx);
        void load(RegisterIndex reg, byte_t val);
        byte_t get_indirect(byte_t page, byte_t idx, byte_t post_fetch = 0);
        byte_t get_indirect_word(word_t idx);

        MOS6502();
        bool cycle();
    };
}

#endif
