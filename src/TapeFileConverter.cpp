#include "TapeFileConverter.h"

#include <cstdint>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>

namespace TapeFileConverter {

    void text_to_binary(const std::string& text_file_path, const std::string& binary_file_path)
    {
        std::ifstream text_file(text_file_path);

        if (!text_file.is_open()) {
            throw std::runtime_error("Failed to open text input file: " + text_file_path);
        }

        std::ofstream binary_file(binary_file_path, std::ios::binary | std::ios::trunc);

        if (!binary_file.is_open()) {
            throw std::runtime_error("Failed to create binary file: " + binary_file_path);
        }

        long long value = 0;

        while (text_file >> value) {
            if (value < std::numeric_limits< std::int32_t >::min() || value > std::numeric_limits< std::int32_t >::max()) {
                throw std::runtime_error("Text file contains value outside int32_t range");
            }

            const std::int32_t binary_value = static_cast< std::int32_t >(value);

            binary_file.write(reinterpret_cast< const char* >(&binary_value), sizeof(binary_value));

            if (!binary_file) {
                throw std::runtime_error("Failed to write value to binary file: " + binary_file_path);
            }
        }

        if (!text_file.eof()) {
            throw std::runtime_error("Text input file contains invalid non-integer data");
        }
    }

    void binary_to_text(const std::string& binary_file_path, const std::string& text_file_path)
    {
        std::ifstream binary_file(binary_file_path, std::ios::binary);

        if (!binary_file.is_open()) {
            throw std::runtime_error("Failed to open binary file: " + binary_file_path);
        }

        binary_file.seekg(0, std::ios::end);

        if (!binary_file) {
            throw std::runtime_error("Failed to move to end of binary file: " + binary_file_path);
        }

        const std::streampos file_size_bytes = binary_file.tellg();

        if (file_size_bytes == std::streampos(-1)) {
            throw std::runtime_error("Failed to determine binary file size: " + binary_file_path);
        }

        const std::streamoff file_size_in_bytes = static_cast< std::size_t >(file_size_bytes);

        if (file_size_in_bytes % sizeof(std::int32_t) != 0) {
            throw std::runtime_error("Binary file size is not divisible by sizeof(int32_t): " + binary_file_path);
        }

        binary_file.clear();
        binary_file.seekg(0, std::ios::beg);

        if (!binary_file) {
            throw std::runtime_error("Failed to return binary file pointer to the beginning");
        }

        std::ofstream text_file(text_file_path, std::ios::trunc);

        if (!text_file.is_open()) {
            throw std::runtime_error("Failed to create text output file: " + text_file_path);
        }
        
        const std::size_t values_count = static_cast< std::size_t >(file_size_in_bytes / sizeof(std::int32_t));

        std::int32_t value = 0;
        binary_file.read(reinterpret_cast< char* >(&value), sizeof(value));

        if (!binary_file) {
            throw std::runtime_error("Failed to read value from binary file: " + binary_file_path);
        }
        if (values_count == 0) {
            text_file << '\n';
            return;
        }
        
        text_file << value;
        
        for (std::size_t i = 0; i < values_count - 1; ++i) {
            text_file << ' ';

            binary_file.read(reinterpret_cast<char*>(&value), sizeof(value));

            if (!binary_file) {
                throw std::runtime_error("Failed to read value from binary file: " + binary_file_path);
            }

            text_file << value;

        }

        text_file << '\n';

        if (!text_file) {
            throw std::runtime_error("Failed to write values to text file: " + text_file_path);
        }
    }

}
