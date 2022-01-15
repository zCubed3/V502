#include "assembler.hpp"

#include <sstream>
#include <iostream>
#include <algorithm>

#include "../components/mos6502.hpp"

namespace V502 {
    enum class RhsType {
        Unknown,
        Address,
        HexNumber,
        BinaryNumber,
        DecNumber
    };

    Assembler6502::Assembler6502(std::string path) {
        std::ifstream file(path);

        if (!file.is_open())
            throw std::runtime_error("Failed to open assembly file!");

        std::stringstream stream;
        stream << file.rdbuf();
        assembly = std::string(stream.str());

        file.close();
    }

    std::vector<uint8_t> Assembler6502::Compile() {
        if (assembly.empty())
            throw std::runtime_error("Can't compile nothing!");

        // TODO: Better malformed code handling

        // First break up the lines
        std::vector<std::string> lines;
        std::stringstream asm_stream(assembly);
        std::string buffer;

        //https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c
        while (std::getline(asm_stream, buffer, '\n'))
            lines.emplace_back(std::string(buffer));

        // We then have to split the lines again, this time by tokens, usually 6502 asm only has a lhs and rhs
        std::vector<std::pair<std::string, std::string>> pairs;

        for (auto line : lines) {
            std::stringstream line_stream(line);
            buffer.clear();

            std::string args[2];
            int s = 0;
            while (std::getline(line_stream, buffer, ' ')) {
                args[s] = std::string(buffer);
                s = !s;
            }

            pairs.emplace_back(std::make_pair(args[0], args[1]));
        }

        // Then we have to determine what operation it is, and what variant of said instruction it is (now, ind_x, ind_y, zpg...)
        std::vector<uint8_t> bytes;
        for (auto pair : pairs) {
            // TODO: Add more instructions
            std::string lhs = pair.first;
            std::transform(lhs.begin(), lhs.end(), lhs.begin(), toupper);

            std::string rhs = pair.second;

            char ident = rhs[0];
            rhs = rhs.substr(1);

            RhsType type = RhsType::Unknown;

            if (ident == '#') {
                char ident2 = rhs[0];
                bool trim = true;

                switch (ident2) {
                    case '$': // Hex number
                        type = RhsType::HexNumber;
                        break;

                    case '%': // Binary number
                        type = RhsType::BinaryNumber;
                        break;

                    default: // Decimal number
                        type = RhsType::DecNumber;
                        trim = false;
                        break;
                }

                if (trim)
                    rhs = rhs.substr(1);
            }
            else if (ident == '$') {
                type = RhsType::Address;
            }

            // TODO: Replace me with something better and less hacky!
            MOS6502::OpCode opcode;
            bool wide = rhs.length() > 2;

            if (lhs == "STA") {
                if (wide)
                    opcode = MOS6502::OpCode::STA;
                else
                    opcode = MOS6502::OpCode::STA_ZPG;
            }

            if (lhs == "ADC") {
                opcode = MOS6502::OpCode::ADC_NOW;
            }

            if (lhs == "JMP") {
                if (!wide)
                    throw std::runtime_error("JMP doesn't support 8 bit addresses!");

                opcode = MOS6502::OpCode::JMP_ABS;
            }

            bytes.emplace_back((uint8_t)opcode);

            for (int x = 0; x < rhs.length(); x += 2) {
                int radix = 10;

                switch (type) {
                    case RhsType::HexNumber:
                    case RhsType::Address:
                        radix = 16;
                        break;

                    case RhsType::BinaryNumber:
                        radix = 2;
                }

                std::string tok = rhs.substr(x, 2);
                bytes.emplace_back(std::stoi(tok, 0, radix));
            }
        }

        return bytes;
    }
}