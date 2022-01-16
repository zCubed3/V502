#include "assembler.hpp"

#include <sstream>
#include <iostream>
#include <algorithm>

#include "../components/mos6502.hpp"

#include "../operations/operation.hpp"

namespace V502 {
    enum RhsType : uint32_t {
        Unknown,
        Address,
        HexNumber,
        BinaryNumber,
        DecNumber
    };

    enum IndexingFlags : uint32_t {
        Indirect = 1,
        IndexedX = 2,
        IndexedY = 4
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

            auto comma = rhs.find(",");
            uint32_t indexing = 0;

            if (comma != std::string::npos) { // Is this indexed?
                char indexer = rhs.substr(comma)[1];
                rhs = rhs.substr(0, comma);

                if (indexer == 'X')
                    indexing |= IndexingFlags::IndexedX;

                if (indexer == 'Y')
                    indexing |= IndexingFlags::IndexedY;
            }

            // TODO: Replace me with something better and less hacky!
            OpCode opcode;
            bool wide = rhs.length() > 2;

            // TODO: Is there a better way to do this?

            if (lhs == "STA") {
                if (wide)
                    opcode = OpCode::STA_ABS;
                else {
                    if (indexing & IndexingFlags::IndexedX)
                        opcode = OpCode::STA_X_ZPG;
                    else
                        opcode = OpCode::STA_ZPG;
                }
            }

            if (lhs == "LDX") {
                opcode = OpCode::LDX_NOW;
            }

            if (lhs == "ADC") {
                opcode = OpCode::ADC_NOW;
            }

            if (lhs == "CMP") {
                opcode = OpCode::CMP_NOW;
            }

            if (lhs == "CPX") {
                opcode = OpCode::CPX_NOW;
            }

            if (lhs == "BEQ") {
                opcode = OpCode::BEQ;
            }

            if (lhs == "JMP") {
                if (!wide)
                    throw std::runtime_error("JMP doesn't support 8 bit addresses!");

                opcode = OpCode::JMP_ABS;
            }

            if (lhs == "INX")
                opcode = OpCode::INX;

            if (lhs == "INY")
                opcode = OpCode::INY;

            bytes.emplace_back(opcode);

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