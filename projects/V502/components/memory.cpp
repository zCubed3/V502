#include "memory.hpp"

#include <fstream>
#include <cstring>

namespace V502 {
    Memory::Memory(uint16_t desired_size) {
        buffer = new uint8_t[desired_size];
        length = desired_size;

        memset(buffer, 0, length);
    }

    void Memory::copy_from(std::ifstream &file) {
        file.seekg(0, std::ifstream::end);
        size_t len = file.tellg();
        file.seekg(0, std::ifstream::beg);

        for (auto b = 0; b < length; b++) {
            if (b > len)
                break;

            file.read(reinterpret_cast<char *>(&buffer[b]), 1);
        }
    }

    byte_t &Memory::at(word_t idx) { return (*this)[idx]; }
    word_t Memory::size() { return length; }
}
