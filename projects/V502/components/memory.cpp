#include "memory.hpp"

namespace V502 {
    Memory::Memory(uint16_t desired_size) {
        buffer = new uint8_t[desired_size];
        size = desired_size;
    }

    uint8_t &Memory::at(uint16_t idx) { return (*this)[idx]; }
}
