#include "TapeSorter.h"

#include "FileTape.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

    const std::filesystem::path temp_work_dir = std::filesystem::path("tmp") / "sort_work";

    void prepare_temp_work_dir()
    {
        std::filesystem::remove_all(temp_work_dir);
        std::filesystem::create_directories(temp_work_dir);
    }

    void remove_temp_work_dir()
    {
        std::filesystem::remove_all(temp_work_dir);
    }

    void transfer_current_value(
        FileTape& source_tape,
        std::size_t& source_tape_processed,
        bool& source_tape_has_value,
        std::int32_t& source_value,
        FileTape& merged_tape,
        std::size_t& merged_tape_written,
        std::size_t merged_tape_size
    )
    {
        merged_tape.write(source_value);

        ++merged_tape_written;
        ++source_tape_processed;

        if (source_tape_processed < source_tape.size()) {
            if (!source_tape.move_right()) {
                throw std::runtime_error("Failed to move temporary tape while merging");
            }

            source_value = source_tape.read();
        } else {
            source_tape_has_value = false;
        }

        if (merged_tape_written < merged_tape_size) {
            if (!merged_tape.move_right()) {
                throw std::runtime_error("Failed to move merged tape while merging");
            }
        }
    }

}

TapeSorter::TapeSorter(const TapeConfig& config):
    config_(config)
{}

std::size_t TapeSorter::calc_block_size() const
{
    const std::size_t element_size = sizeof(std::int32_t);

    if (config_.m_limit_bytes < element_size) {
        throw std::runtime_error("Memory limit is too small to store even one int32_t value");
    }

    return config_.m_limit_bytes / element_size;
}

void TapeSorter::sort(ITape& input_tape, ITape& output_tape)
{
    if (input_tape.size() != output_tape.size()) {
        throw std::runtime_error("Input and output tapes have different sizes");
    }

    if (input_tape.size() == 0) {
        return;
    }

    prepare_temp_work_dir();

    try {
        input_tape.rewind();
        output_tape.rewind();

        std::vector< std::string > temporary_tape_paths = create_sorted_temp_tapes(input_tape);

        const std::string sorted_tape_path = merge_temp_tapes(temporary_tape_paths);

        copy_sorted_tape_to_output(sorted_tape_path, output_tape);
    } catch (...) {
        remove_temp_work_dir();
        throw;
    }

    remove_temp_work_dir();
}

std::vector< std::string > TapeSorter::create_sorted_temp_tapes(ITape& input_tape) const
{
    const std::size_t block_size = calc_block_size();
    const std::size_t input_size = input_tape.size();

    std::vector< std::string > temporary_tape_paths;

    std::size_t processed_elements = 0;
    std::size_t block_index = 0;

    input_tape.rewind();

    while (processed_elements < input_size) {
        const std::size_t remaining_elements = input_size - processed_elements;
        const std::size_t current_block_size = std::min(block_size, remaining_elements);

        std::vector< std::int32_t > block;
        block.reserve(current_block_size);

        for (std::size_t i = 0; i < current_block_size; ++i) {
            block.push_back(input_tape.read());
            ++processed_elements;

            if (processed_elements < input_size) {
                if (!input_tape.move_right()) {
                    throw std::runtime_error("Failed to move input tape while reading block");
                }
            }
        }

        std::sort(block.begin(), block.end());

        const std::string temporary_tape_path = (temp_work_dir / ("block_" + std::to_string(block_index) + ".bin")).string();

        FileTape temporary_tape(temporary_tape_path, block.size(), config_);

        for (std::size_t i = 0; i < block.size(); ++i) {
            temporary_tape.write(block[i]);

            if (i + 1 < block.size()) {
                if (!temporary_tape.move_right()) {
                    throw std::runtime_error("Failed to move temporary tape while writing sorted block");
                }
            }
        }

        temporary_tape_paths.push_back(temporary_tape_path);
        ++block_index;
    }

    return temporary_tape_paths;
}

std::string TapeSorter::merge_temp_tapes(std::vector< std::string > temporary_tape_paths) const
{
    if (temporary_tape_paths.empty()) {
        throw std::runtime_error("No temporary tapes were created");
    }

    std::size_t merge_index = 0;

    while (temporary_tape_paths.size() > 1) {
        std::vector< std::string > next_level_tapes;

        for (std::size_t i = 0; i < temporary_tape_paths.size(); i += 2) {
            if (i + 1 < temporary_tape_paths.size()) {
                const std::string merged_tape_path = merge_two_tapes(
                        temporary_tape_paths[i],
                        temporary_tape_paths[i + 1],
                        merge_index
                    );

                next_level_tapes.push_back(merged_tape_path);
                ++merge_index;
            } else {
                next_level_tapes.push_back(temporary_tape_paths[i]);
            }
        }

        temporary_tape_paths = next_level_tapes;
    }

    return temporary_tape_paths[0];
}

std::string TapeSorter::merge_two_tapes(
    const std::string& first_tape_path,
    const std::string& second_tape_path,
    std::size_t merge_index
) const
{
    FileTape first_tape(first_tape_path, config_);
    FileTape second_tape(second_tape_path, config_);

    const std::size_t merged_tape_size = first_tape.size() + second_tape.size();

    const std::string merged_tape_path = (temp_work_dir / ("merge_" + std::to_string(merge_index) + ".bin")).string();

    FileTape merged_tape(merged_tape_path, merged_tape_size, config_);

    first_tape.rewind();
    second_tape.rewind();
    merged_tape.rewind();

    std::size_t first_tape_processed = 0;
    std::size_t second_tape_processed = 0;
    std::size_t merged_tape_written = 0;

    bool first_tape_has_value = first_tape.size() > 0;
    bool second_tape_has_value = second_tape.size() > 0;

    std::int32_t first_value = 0;
    std::int32_t second_value = 0;

    if (first_tape_has_value) {
        first_value = first_tape.read();
    }

    if (second_tape_has_value) {
        second_value = second_tape.read();
    }

    while (first_tape_has_value && second_tape_has_value) {
        if (first_value <= second_value) {
            transfer_current_value(
                first_tape,
                first_tape_processed,
                first_tape_has_value,
                first_value,
                merged_tape,
                merged_tape_written,
                merged_tape_size
            );
        } else {
            transfer_current_value(
                second_tape,
                second_tape_processed,
                second_tape_has_value,
                second_value,
                merged_tape,
                merged_tape_written,
                merged_tape_size
            );
        }
    }

    while (first_tape_has_value) {
        transfer_current_value(
            first_tape,
            first_tape_processed,
            first_tape_has_value,
            first_value,
            merged_tape,
            merged_tape_written,
            merged_tape_size
        );
    }

    while (second_tape_has_value) {
        transfer_current_value(
            second_tape,
            second_tape_processed,
            second_tape_has_value,
            second_value,
            merged_tape,
            merged_tape_written,
            merged_tape_size
        );
    }

    return merged_tape_path;
}

void TapeSorter::copy_sorted_tape_to_output(const std::string& sorted_tape_path, ITape& output_tape) const
{
    FileTape sorted_tape(sorted_tape_path, config_);

    if (sorted_tape.size() != output_tape.size()) {
        throw std::runtime_error("Sorted temporary tape and output tape have different sizes");
    }

    sorted_tape.rewind();
    output_tape.rewind();

    for (std::size_t i = 0; i < sorted_tape.size(); ++i) {
        output_tape.write(sorted_tape.read());

        if (i + 1 < sorted_tape.size()) {
            if (!sorted_tape.move_right()) {
                throw std::runtime_error("Failed to move sorted temporary tape while copying to output");
            }

            if (!output_tape.move_right()) {
                throw std::runtime_error("Failed to move output tape while copying sorted values");
            }
        }
    }
}
