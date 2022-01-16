#include "memory.hpp"

#include <fstream>

namespace V502 {
    Memory::Memory(uint16_t desired_size) {
        buffer = new uint8_t[desired_size];
        length = desired_size;
    }

    Memory::Memory(std::ifstream &file) {
        file.seekg(0, std::ifstream::end);
        size_t len = file.tellg();
        file.seekg(0, std::ifstream::beg);

        buffer = new uint8_t[len];
        length = len;

        for (auto b = 0; b < len; b++)
            file.read(reinterpret_cast<char*>(&buffer[b]), 1);
    }

    byte_t &Memory::at(word_t idx) { return (*this)[idx]; }
    word_t Memory::size() { return length; }
}
