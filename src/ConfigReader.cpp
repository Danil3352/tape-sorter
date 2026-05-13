#include "ConfigReader.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <map>
#include <vector>

TapeConfig ConfigReader::read_config(const std::string& file_path)
{
    std::ifstream input_file(file_path);

    if (!input_file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + file_path);
    }

    std::vector< bool > vec_check_exist(5, 0);
    std::vector< std::size_t > data(5, 0);
    std::map< std::string, std::size_t > val_for_str {
        {"read_delay_ms", 0},
        {"write_delay_ms", 1},
        {"move_delay_ms", 2},
        {"rewind_delay_ms", 3},
        {"m_limit_bytes", 4}
    };
    std::string line;
    while(std::getline(input_file, line))
    {
        if (line.empty()) {
            continue;
        }

        const std::size_t sep_pos = line.find('=');

        if (sep_pos == std::string::npos) {
            throw std::runtime_error("Invalid config line: " + line);
        }

        const std::string key = line.substr(0, sep_pos);
        const std::string str_value = line.substr(sep_pos + 1);

        std::size_t num_value;

        try {
            num_value = std::stoull(str_value);
        } catch (const std::exception&) {
            throw std::runtime_error("Invalid numeric value: " + line);
        }
        const auto it = val_for_str.find(key);
        if (it != val_for_str.end()) {
            const std::size_t index = it->second;
            if (vec_check_exist[index] == 0) {
                data[index] = num_value;
                vec_check_exist[index] = true;
            } else {
                throw std::runtime_error("Double input key: " + key);
            }

        } else {
            throw std::runtime_error("Unknown config key: " + key);
        }
    }

    if (std::find(vec_check_exist.begin(), vec_check_exist.end(), false) != vec_check_exist.end()) {
        throw std::runtime_error("Missing config key");
    }

    return TapeConfig{
        data[0],
        data[1],
        data[2],
        data[3],
        data[4]
    };
}
