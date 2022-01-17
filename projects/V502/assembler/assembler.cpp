#include "assembler.hpp"

#include <sstream>
#include <iostream>
#include <algorithm>

#include <cstring>

#include "instruction_container.hpp"

#include <components/mos6502.hpp>
#include <operations/operation.hpp>

#define ASM_BASE_ORIGIN 0x4000

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
        std::vector<word_t> line_numbers;
        std::stringstream asm_stream(assembly);
        std::string buffer;

        //https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c
        int number = 0;
        while (std::getline(asm_stream, buffer, '\n')) {
            number += 1;

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

            // Trim tabs
            while (buffer[0] == '\t')
                buffer = buffer.substr(1);

            std::transform(buffer.begin(), buffer.end(), buffer.begin(), toupper); // Converts everything to uppercase
            lines.emplace_back(buffer);
            line_numbers.emplace_back(number);
        }

        // We then have to split the lines again, this time by tokens, usually 6502 asm only has a lhs and rhs
        std::vector<std::tuple<uint16_t, std::string, std::string>> tokens;
        word_t i = 0;
        for (auto line : lines) {
            std::stringstream line_stream(line);
            buffer.clear();

            std::string args[2];
            int s = 0;
            while (std::getline(line_stream, buffer, ' ')) {
                args[s] = std::string(buffer);
                s = !s;
            }

            tokens.emplace_back(std::make_tuple(line_numbers[i++], args[0], args[1]));
        }

        word_t origin = ASM_BASE_ORIGIN; // Default program location
        word_t write = origin;
        std::vector<uint8_t> bytes(0xFFFF); // 65535 bytes long, we generate giant rom images!
        memset(bytes.data(), 0, bytes.size());

        // Preprocessors and labels
        bool org_changed = false;

        // symbol, line number, offset
        std::unordered_map<std::string, std::pair<uint16_t, std::optional<uint16_t>>> labels; // If the optional is missing a value, this label hasn't been resolved

        for (auto token : tokens) {
            std::string lhs = std::get<1>(token);
            std::string rhs = std::get<2>(token);
            word_t line = std::get<0>(token);

            if (lhs[0] == '.') { // Is this a preprocessor directive?
                lhs = lhs.substr(1);
                if (lhs == "ORG") {
                    if (org_changed) {
                        std::cerr << "Line " << line << ": Multiple .org directives are not allowed!" << std::endl;
                        continue;
                    }

                    try {
                        write = origin = std::stoi(rhs, 0, 16);
                        std::cout << "Explicit origin was provided, " << std::hex << origin << std::endl;
                    } catch (std::exception& err) {
                        std::cerr << "Line " << line << ": .org was provided an invalid origin address, valid inputs look like '4000'!" << std::endl;
                        std::cerr << "Falling back to default origin of " << std::hex << ASM_BASE_ORIGIN << std::endl;
                    }

                    org_changed = true;
                }
            }

            if (lhs.back() == ':') {
                lhs.pop_back(); // Removes colon
                labels.emplace(lhs, std::make_pair(line, std::optional<word_t>()));
            }
        }

        if (!org_changed)
            std::cout << "Explicit origin not provided, '.org', using default origin " << std::hex << ASM_BASE_ORIGIN << std::endl;

        // We tell the system where the program begins
        bytes[0xFFFC] = origin >> 8;
        bytes[0xFFFD] = origin;

        std::vector<std::tuple<uint16_t, std::string, uint16_t>> unresolved_tokens;

        // Then we have to determine what operation it is, and what variant of said instruction it is (now, ind_x, ind_y, zpg...)
        // If an instruction relies on a label, it gets pushed into the post processor
        for (auto token : tokens) {
            std::string lhs = std::get<1>(token);
            std::string rhs = std::get<2>(token);
            word_t line = std::get<0>(token);

            char ident = rhs.length() > 0 ? rhs[0] : 0;

            if (ident != 0 && ident == '$' || ident == '#')
                rhs = rhs.substr(1);

            RhsType type = RhsType::Unknown;
            uint32_t calling = 0;

            if (lhs[0] == '.') { // Is this a preprocessor directive?
                continue;
            }

            if (lhs.back() == ':') { // Is this a label?
                lhs.pop_back();
                labels[lhs].second = write;
                continue;
            }

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
            } else if (ident == '$') {
                calling |= CallingFlags::ZeroPage; // If this is a byte
                type = RhsType::Address;
            } else if (!rhs.empty()) { // This is most likely a label
                unresolved_tokens.emplace_back(std::make_tuple(line, rhs, write + 1));
                rhs = "FFFF"; // Placeholder
                type = RhsType::HexNumber;
            }

            OptOpCode opcode;
            bool word = rhs.length() > 2;

            for (auto container: InstructionContainer::containers) {
                if (container.symbol == lhs) {
                    auto opt = container.get_code(calling, word);
                    if (opt.has_value())
                        opcode = opt;
                    else {
                        std::cerr << "Line " << line << ": " << lhs
                                  << " is an instruction, but the variant you're trying to use is invalid, please check your syntax!"
                                  << std::endl;
                        throw std::runtime_error("Instruction was recognized but variant was not!");
                    }
                }
            }

            if (!opcode.has_value()) {
                std::cerr << "Line " << line << ": " << lhs << " is not a valid instruction, it either isn't recognized or implemented!"
                          << std::endl;
                throw std::runtime_error("Unknown instruction!");
            }

            bytes[write++] = opcode.value();

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
                    bytes[write++] = std::stoi(tok, 0, radix);
                }
            }
        }

        // Post processing, for resolved labels
        for (auto token : unresolved_tokens) {
            word_t line = std::get<0>(token);
            std::string target = std::get<1>(token);
            word_t offset = std::get<2>(token);

            if (labels.count(target) != 0) {
                auto resolve = labels[target].second;

                if (!resolve.has_value()) {
                    std::cerr << "Line " << line << ": '" << target << "' was unresolved!" << std::endl;
                    throw std::runtime_error("Label is unresolved!");
                }

                word_t final = resolve.value();

                bytes[offset] = final >> 8;
                bytes[offset + 1] = final;
            } else {
                std::cerr << "Line " << line << ": Unknown token '"<< target << "'! Did you forget to define a label?" << std::endl;
                throw std::runtime_error("Unknown token for label!");
            }
        }

        return bytes;
    }
}