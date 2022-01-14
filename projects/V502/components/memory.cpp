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

    uint8_t &Memory::at(uint16_t idx) { return (*this)[idx]; }
    uint16_t Memory::size() { return length; }
}
