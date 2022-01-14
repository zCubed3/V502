#include <V502/components/mos6502.hpp>
#include <V502/components/memory.hpp>

#include <iostream>
#include <fstream>

int main() {
    V502::MOS6502* cpu = new V502::MOS6502();

    V502::Memory* sys_memory = new V502::Memory(255);
    cpu->system_memory = sys_memory;

    // If autorun.bin is detected, run it, otherwise ask
    std::ifstream binfile("autorun.bin");
    char path[256];

    while (!binfile.is_open()) {
        std::cout << "Path to bin? ";
        std::cin >> path;

        binfile.open(path);

        if (!binfile.is_open())
            std::cout << "bin file not found at '" << path << "'" << std::endl;
    }

    binfile.seekg(0, std::ifstream::end);
    size_t binsize = binfile.tellg();
    binfile.seekg(0, std::ifstream::beg);

    V502::Memory* prog_memory = new V502::Memory(binsize);
    cpu->program_memory = prog_memory;

    for (auto b = 0; b < binsize; b++) {
        binfile.read(reinterpret_cast<char*>(&prog_memory->at(b)), 1);
    }

    binfile.close();

    std::cout << "Binary data: " << std::endl;
    for (auto d = 0; d < prog_memory->size(); d++) {
        std::cout << std::hex << +prog_memory->at(d) << " ";

        if (d % 16 == 0 && d != 0)
            std::cout << std::endl;
    }

    std::cout << std::dec << std::endl;

    while (cpu->Cycle()) {
        std::cout << std::hex << "0x0000: " << +sys_memory->at(0) << "\r" << std::flush;

        //for (int x = 0; x < 8; x++) {
        //    printf("%i", (cpu->flags << x) & 1);
        //}

        //printf("\n");
    }

    return 0;
}