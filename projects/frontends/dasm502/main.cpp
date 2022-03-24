#define V502_INCLUDE_ASSEMBLER
#include <v502/v502.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

int main() {
    v502_assembler_instance_t *assembler = v502_create_assembler();

    std::ifstream bin_file("test.bin");

    v502_binary_file_t* binary = (v502_binary_file_t*)calloc(1, sizeof(v502_binary_file_t));

    binary->length = 0xFFFF + 1;
    binary->bytes = (char*)malloc(binary->length);

    bin_file.read(reinterpret_cast<char*>(binary->bytes), binary->length);

    const char* out = v502_disassemble_binary(assembler, binary);

    std::ofstream out_file("test.s", std::ofstream::binary | std::ofstream::trunc);
    out_file << out << std::endl;
    out_file.close();

    return 0;
}