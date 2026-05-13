#include "FileTape.h"

#include <chrono>
#include <fstream>
#include <stdexcept>
#include <thread>

void FileTape::open_created_tape(const std::string& file_path)
{
    file_.open(file_path, std::ios::in | std::ios::out | std::ios::binary);

    if (!file_.is_open()) {
        throw std::runtime_error("Failed to open created tape file: " + file_path);
    }
}

std::size_t FileTape::open_existing_tape_and_get_size(const std::string& file_path)
{
    open_created_tape(file_path);

    file_.seekg(0, std::ios::end);

    if (!file_) {
        throw std::runtime_error("Failed to move to end of tape file: " + file_path);
    }

    const std::streampos file_size_bytes = file_.tellg();

    if (file_size_bytes == std::streampos(-1)) {
        throw std::runtime_error("Failed to determine tape file size: " + file_path);
    }

    const std::size_t file_size_in_bytes = static_cast< std::size_t >(file_size_bytes);

    if (file_size_in_bytes % sizeof(std::int32_t) != 0) {
        throw std::runtime_error("Tape file size is not divisible by sizeof(int32_t): " + file_path);
    }

    file_.clear();
    file_.seekg(0, std::ios::beg);
    file_.seekp(0, std::ios::beg);

    if (!file_) {
        throw std::runtime_error("Failed to return tape file pointers to the beginning: " + file_path);
    }

    return static_cast< std::size_t >(file_size_in_bytes / sizeof(std::int32_t));
}

void FileTape::create_empty_tape_file(const std::string& file_path, std::size_t tape_size)
{
    std::ofstream output_file(file_path, std::ios::binary | std::ios::trunc);

    if (!output_file.is_open()) {
        throw std::runtime_error("Failed to create tape file: " + file_path);
    }

    const std::int32_t zero_value = 0;

    for (std::size_t i = 0; i < tape_size; ++i) {
        output_file.write(reinterpret_cast< const char* >(&zero_value), sizeof(zero_value));

        if (!output_file) {
            throw std::runtime_error("Failed to initialize tape file: " + file_path);
        }
    }
}


FileTape::FileTape(const std::string& file_path, const TapeConfig& config):
    file_{},
    config_(config),
    tape_size_(open_existing_tape_and_get_size(file_path)),
    current_position_(0)
{}

FileTape::FileTape(const std::string& file_path, std::size_t tape_size, const TapeConfig& config):
    file_{},
    config_(config),
    tape_size_(tape_size),
    current_position_(0)
{
    create_empty_tape_file(file_path, tape_size_);
    open_created_tape(file_path);
}

std::int32_t FileTape::read()
{
    check_tape_is_not_empty();

    file_.clear();
    file_.seekg(current_byte_offset(), std::ios::beg);

    if (!file_) {
        throw std::runtime_error("Failed to move read pointer in tape file");
    }

    std::int32_t value = 0;

    file_.read(reinterpret_cast< char* >(&value), sizeof(value));

    if (!file_) {
        throw std::runtime_error("Failed to read value from tape file");
    }

    apply_delay(config_.read_delay_ms);

    return value;
}

void FileTape::write(std::int32_t value)
{
    check_tape_is_not_empty();

    file_.clear();
    file_.seekp(current_byte_offset(), std::ios::beg);

    if (!file_) {
        throw std::runtime_error("Failed to move write pointer in tape file");
    }

    file_.write(reinterpret_cast< const char* >(&value), sizeof(value));

    if (!file_) {
        throw std::runtime_error("Failed to write value to tape file");
    }

    apply_delay(config_.write_delay_ms);
}

bool FileTape::move_left()
{
    if (tape_size_ == 0 || current_position_ == 0) {
        return false;
    }

    --current_position_;

    apply_delay(config_.move_delay_ms);

    return true;
}

bool FileTape::move_right()
{
    if (tape_size_ == 0 || current_position_ + 1 >= tape_size_) {
        return false;
    }

    ++current_position_;

    apply_delay(config_.move_delay_ms);

    return true;
}

void FileTape::rewind()
{
    current_position_ = 0;

    apply_delay(config_.rewind_delay_ms);
}

std::size_t FileTape::size() const
{
    return tape_size_;
}

std::size_t FileTape::position() const
{
    return current_position_;
}

std::streamoff FileTape::current_byte_offset() const
{
    return static_cast< std::streamoff >(current_position_ * sizeof(std::int32_t));
}

void FileTape::apply_delay(std::size_t delay_ms) const
{
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
}

void FileTape::check_tape_is_not_empty() const
{
    if (tape_size_ == 0) {
        throw std::runtime_error("Cannot read or write empty tape");
    }
}
