#include <iostream>
#include <stdexcept>

#include "ConfigReader.h"

int main()
{
    try {
        const TapeConfig config = ConfigReader::read_config("config.txt");

        std::cout << "read_delay_ms = " << config.read_delay_ms << '\n';
        std::cout << "write_delay_ms = " << config.write_delay_ms << '\n';
        std::cout << "move_delay_ms = " << config.move_delay_ms << '\n';
        std::cout << "rewind_delay_ms = " << config.rewind_delay_ms << '\n';
        std::cout << "m_limit_bytes = " << config.m_limit_bytes << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
