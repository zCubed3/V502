#include <V502/components/mos6502.hpp>
#include <V502/components/memory.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <V502/assembler/assembler.hpp>

// Frontend for the assembler
int main() {
    std::string str;

    std::cout << "Where is your source (.s) file?" << std::endl;
    std::cin >> str;

    V502::Assembler6502 *assembler = new V502::Assembler6502(str);
    std::vector<uint8_t> bytes = assembler->compile();

    str.clear();

    std::cout << "Where should the binary go?" << std::endl;
    std::cin >> str;

    std::ofstream out(str);
    out.write(reinterpret_cast<char*>(bytes.data()), bytes.size());
    out.close();

    return 0;
}