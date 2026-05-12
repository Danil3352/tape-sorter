#ifndef TAPECONFIG_H
#define TAPECONFIG_H

#include <cstddef>

struct TapeConfig
{
    std::size_t read_delay_ms;
    std::size_t write_delay_ms;
    std::size_t move_delay_ms;
    std::size_t rewind_delay_ms;
    std::size_t m_limit_bytes;

    static TapeConfig from_file(const std::string& file_path);
};

#endif
