#ifndef TAPE_SORTER_H
#define TAPE_SORTER_H

#include "ITape.h"
#include "TapeConfig.h"

#include <cstddef>
#include <string>
#include <vector>

class TapeSorter {
public:
    TapeSorter(const TapeConfig& config);

    void sort(ITape& input_tape, ITape& output_tape);

private:
    TapeConfig config_;

    std::size_t calc_block_size() const;

    std::vector< std::string > create_sorted_temp_tapes(ITape& input_tape) const;

    std::string merge_temp_tapes(std::vector< std::string > temporary_tape_paths) const;

    std::string merge_two_tapes(const std::string& first_tape_path, const std::string& second_tape_path, 
        std::size_t merge_index) const;

    void copy_sorted_tape_to_output(const std::string& sorted_tape_path, ITape& output_tape) const;
};

#endif
