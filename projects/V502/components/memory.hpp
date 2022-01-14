#ifndef V502_MEMORY_HPP
#define V502_MEMORY_HPP

#include <stdint.h>
#include <stdexcept>
#include <iostream>

namespace V502 {
    class Memory {
        // All memory really is a giant uint8_t block!
        uint8_t *buffer;
        uint16_t length;

    public:
        Memory(uint16_t desired_size);

        uint8_t& at(uint16_t idx);

        uint8_t& operator[](uint16_t idx) {
            if (idx >= length) {
                std::cerr << "[memory.hpp] Attempted to index memory at " << idx << std::endl;
                std::cerr << "[memory.hpp] Valid index range is 0 to " << length - 1 << std::endl;
                throw new std::runtime_error("Attempted to index memory outside of memory block!");
            }

            return buffer[idx];
        }

        uint16_t size();
    };
}

#endif
