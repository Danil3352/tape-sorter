#ifndef FILE_TAPE_H
#define FILE_TAPE_H

#include "ITape.h"
#include "TapeConfig.h"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>

class FileTape : public ITape
{
public:
    FileTape(const std::string& file_path, const TapeConfig& config);
    FileTape(const std::string& file_path, std::size_t tape_size, const TapeConfig& config);

    std::int32_t read() override;
    void write(std::int32_t value) override;

    bool move_left() override;
    bool move_right() override;

    void rewind() override;

    std::size_t size() const override;
    std::size_t position() const override;

private:
    std::fstream file_;
    TapeConfig config_;
    std::size_t tape_size_;
    std::size_t current_position_;

    std::size_t open_existing_tape_and_get_size(const std::string& file_path);
    void create_empty_tape_file(const std::string& file_path, std::size_t tape_size);
    void open_created_tape(const std::string& file_path);

    std::streamoff current_byte_offset() const;

    void apply_delay(std::size_t delay_ms) const;
    void check_tape_is_not_empty() const;
};

#endif
