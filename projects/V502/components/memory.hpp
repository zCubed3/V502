#ifndef V502_MEMORY_HPP
#define V502_MEMORY_HPP

#include <stdint.h>
#include <stdexcept>
#include <iostream>

#include <v502types.hpp>

namespace V502 {
    class Memory {
        // All memory really is a giant uint8_t block!
        byte_t *buffer;
        word_t length;

    public:
        Memory(word_t desired_size);
        Memory(std::ifstream &file);

        byte_t& at(word_t idx);

        byte_t& operator[](word_t idx) {
            if (idx >= length) {
                std::cerr << "[memory.hpp] Attempted to index memory at " << idx << std::endl;
                std::cerr << "[memory.hpp] Valid index range is 0 to " << length - 1 << std::endl;
                throw new std::runtime_error("Attempted to index memory outside of memory block!");
            }

            return buffer[idx];
        }

        word_t size();
    };
}

#endif
