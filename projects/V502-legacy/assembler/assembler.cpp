#include "assembler.hpp"

#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
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

            // Trim tabs and spaces
            while (buffer[0] == '\t' || buffer[0] == ' ')
                buffer = buffer.substr(1);

            // Trim trailing spaces
            while (buffer.back() == ' ')
                buffer.pop_back();

            // If this line starts with ; we discard it, it's a comment
            if (buffer[0] == ';')
                continue;

            auto comment = buffer.find(";");
            if (comment != std::string::npos)
                buffer = buffer.substr(0, comment);

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
        std::vector<uint8_t> bytes(65536); // 65536 bytes long, we generate giant rom images!
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
            std::cout << "Explicit origin not provided, (use '.org' if you want a custom origin), using default origin " << std::hex << ASM_BASE_ORIGIN << std::endl;

        // We tell the system where the program begins
        bytes[0xFFFC] = origin;
        bytes[0xFFFD] = origin >> 8; // Writing words is backwards!

        std::vector<std::tuple<uint16_t, std::string, uint16_t, bool>> unresolved_tokens;

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
                // If it's indirect, correct the ident
                rhs = rhs.substr(1);
                ident = rhs[0];
                rhs = rhs.substr(1);

                auto closing = rhs.find(')');
                if (closing == std::string::npos) {
                    std::cerr << "Line " << line << ": " << lhs << " is lacking a closing parenthesis!"
                              << std::endl;
                    throw std::runtime_error("Open parenthesis!");
                }

                rhs = rhs.erase(closing, 1);
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
                // Check if the instruction marks this as relative
                bool relative = false;

                for (auto container: InstructionContainer::containers) {
                    if (container.symbol == lhs) {
                        relative = container.relative;
                        break;
                    }
                }

                unresolved_tokens.emplace_back(std::make_tuple(line, rhs, write + 1, relative));

                // If the indexer symbols are found, we shorten the placeholder
                bool shorter = relative; // Relative symbols are bytes!
                auto indexer = rhs.find('[');

                if (!shorter)
                    shorter = indexer != std::string::npos && rhs.back() == ']';

                rhs = shorter ? "FF" : "FFFF"; // Placeholder

                type = RhsType::HexNumber;
            }

            OptOpCode opcode;
            bool word = rhs.length() > 2 && (type == RhsType::HexNumber || type == RhsType::Address);

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
                if (type == RhsType::HexNumber || type == RhsType::Address) {
                    std::vector<uint8_t> args;

                    if (type)
                        for (int x = 0; x < rhs.length(); x += 2) {
                            std::string tok = rhs.substr(x, 2);
                            args.emplace_back(std::stoi(tok, 0, 16));
                        }

                    std::reverse(args.begin(), args.end());
                    for (auto arg: args)
                        bytes[write++] = arg;
                } else if (type == RhsType::DecNumber) {
                    bytes[write++] = std::stoi(rhs, 0, 10);
                } else if (type == RhsType::BinaryNumber) {
                    bytes[write++] = std::stoi(rhs, 0, 2);
                } else {
                    std::cerr << "Line " << line << ": argument is unrecognized!" << std::endl;
                    throw std::runtime_error("Argument type wasn't recognized!");
                }
            }
        }

        // Post processing, for resolved labels
        for (auto token : unresolved_tokens) {
            word_t line = std::get<0>(token);
            std::string target = std::get<1>(token);
            word_t offset = std::get<2>(token);
            bool relative = std::get<3>(token);

            // Is there an indexer at the end?
            std::optional<byte_t> label_indexer;
            auto indexer = target.find('[');
            if (indexer != std::string::npos && target.back() == ']') {
                // We need to extract the number in the middle
                char num = target[indexer + 1];

                if (!isdigit(num)) {
                    std::cerr << "Line " << line << ": '" << target << "' has an invalid label indexer!" << std::endl;
                    throw std::runtime_error("Indexer is either empty or has an invalid character!");
                }

                label_indexer = num - '0';

                if (label_indexer > 2) { // TODO: Named arrays?
                    std::cerr << "Line " << line << ": '" << target << "' indexer is out of range, valid indexers are 0 and 1!" << std::endl;
                    throw std::runtime_error("Indexer is out of range!");
                }

                target = target.substr(0, indexer);
            }

            if (labels.count(target) != 0) {
                auto resolve = labels[target].second;

                if (!resolve.has_value()) {
                    std::cerr << "Line " << line << ": '" << target << "' was unresolved!" << std::endl;
                    throw std::runtime_error("Label is unresolved!");
                }

                word_t final = resolve.value();

                // Another gotcha from the 6502, if this is a relative resolve, we have to make sure we can reach it!
                if (relative) {
                    int16_t displace = final - offset;

                    if (displace < -127 || displace > 128) {
                        std::cerr << "Line " << line << ": Branch is too far away from label, due to a limitation of the 6502, branches can only go 127 bytes backward and 128 bytes forward!" << std::endl;
                        throw std::runtime_error("Branch is too far away!");
                    }

                    bytes[offset] = (int8_t)(final - offset);
                } else {
                    if (label_indexer.has_value()) {
                        if (label_indexer == 0)
                            bytes[offset] = final;
                        else
                            bytes[offset] = final >> 8;
                    } else {
                        bytes[offset] = final;
                        bytes[offset + 1] = final >> 8;
                    }
                }
            } else {
                std::cerr << "Line " << line << ": Unknown token '"<< target << "'! Did you forget to define a label?" << std::endl;
                throw std::runtime_error("Unknown token for label!");
            }
        }

        return bytes;
    }
}