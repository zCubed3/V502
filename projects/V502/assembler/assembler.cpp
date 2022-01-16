#include "assembler.hpp"

#include <sstream>
#include <iostream>
#include <algorithm>

#include "instruction_container.hpp"

#include <components/mos6502.hpp>
#include <operations/operation.hpp>

namespace V502 {
    enum RhsType : uint32_t {
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

    std::vector<uint8_t> Assembler6502::compile() {
        if (assembly.empty())
            throw std::runtime_error("Can't compile nothing!");

        // TODO: Better malformed code handling

        // First break up the lines
        std::vector<std::string> lines;
        std::stringstream asm_stream(assembly);
        std::string buffer;

        //https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c
        while (std::getline(asm_stream, buffer, '\n')) {
            if (buffer.empty())
                continue;

            // If this line starts with ; we discard it, it's a comment
            if (buffer[0] == ';')
                continue;

            auto comment = buffer.find(";");
            if (comment != std::string::npos)
                buffer = buffer.substr(0, comment);

            // Trim trailing spaces
            while (buffer.back() == ' ')
                buffer.pop_back();

            std::transform(buffer.begin(), buffer.end(), buffer.begin(), toupper); // Converts everything to uppercase
            lines.emplace_back(buffer);
        }

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
            std::string rhs = pair.second;

            char ident = rhs.length() > 0 ? rhs[0] : 0;

            if (ident != 0)
                rhs = rhs.substr(1);

            RhsType type = RhsType::Unknown;
            uint32_t calling = 0;


            if (ident == '(') { // Is this indirect? (or deferred?)
                // If it's indirect, ident is wrong
                ident = rhs[0];
                rhs = rhs.substr(1);
                rhs.pop_back();

                calling |= CallingFlags::Indirect;
            }

            auto comma = rhs.find(",");
            if (comma != std::string::npos) { // Is this indexed?
                char indexer = rhs.substr(comma)[1];
                rhs = rhs.substr(0, comma);

                if (indexer == 'X')
                    calling |= CallingFlags::IndexedX;

                if (indexer == 'Y')
                    calling |= CallingFlags::IndexedY;
            }

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
                calling |= CallingFlags::ZeroPage; // If this is a byte
                type = RhsType::Address;
            }

            OptOpCode opcode;
            bool word = rhs.length() > 2;

            for (auto container : InstructionContainer::containers) {
                if (container.symbol == lhs) {
                    auto opt = container.get_code(calling, word);
                    if (opt.has_value())
                        opcode = opt;
                    else {
                        std::cerr << lhs << " is an instruction, but the variant you're trying to use is invalid, please check your syntax!" << std::endl;
                        throw std::runtime_error("Instruction was recognized but variant was not!");
                    }
                }
            }

            if (!opcode.has_value()) {
                std::cerr << lhs << " is not a valid instruction, it either isn't recognized or implemented!" << std::endl;
                throw std::runtime_error("Unknown instruction!");
            }

            bytes.emplace_back(opcode.value());

            if (rhs.length() > 0) {
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
        }

        return bytes;
    }
}