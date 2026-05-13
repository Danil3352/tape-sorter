#include "FileTape.h"
#include "TapeSorter.h"

#include <boost/test/unit_test.hpp>

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{

    std::filesystem::path get_test_dir()
    {
        return "tmp/tests_tape_sorter";
    }

    TapeConfig get_test_config(std::size_t m_limit_bytes)
    {
        return TapeConfig{0, 0, 0, 0, m_limit_bytes};
    }

    std::filesystem::path get_file_path(const std::string& file_name)
    {
        std::filesystem::create_directories(get_test_dir());
        return get_test_dir() / file_name;
    }

    void write_values_to_tape(FileTape& tape, const std::vector< std::int32_t >& values)
    {
        tape.rewind();

        for (std::size_t i = 0; i < values.size(); ++i) {
            tape.write(values[i]);

            if (i + 1 < values.size()) {
                if (!tape.move_right()) {
                    throw std::runtime_error("Failed to move tape while writing test values");
                }
            }
        }
    }

    std::vector< std::int32_t > read_values_from_tape(FileTape& tape)
    {
        std::vector< std::int32_t > values;

        tape.rewind();

        for (std::size_t i = 0; i < tape.size(); ++i) {
            values.push_back(tape.read());

            if (i + 1 < tape.size()) {
                if (!tape.move_right()) {
                    throw std::runtime_error("Failed to move tape while reading test values");
                }
            }
        }

        return values;
    }

}

BOOST_AUTO_TEST_CASE(tape_sorter_sorts_values)
{
    std::filesystem::remove_all(get_test_dir());
    {
        const std::vector< std::int32_t > input_values{8, 3, 5, 1, 9, 7, 4, 2};
        const std::vector< std::int32_t > expected_values{1, 2, 3, 4, 5, 7, 8, 9};

        const TapeConfig config = get_test_config(12);

        FileTape input_tape(get_file_path("input.bin").string(), input_values.size(), config);
        FileTape output_tape(get_file_path("output.bin").string(), input_values.size(), config);

        write_values_to_tape(input_tape, input_values);

        TapeSorter sorter(config);
        sorter.sort(input_tape, output_tape);

        const std::vector< std::int32_t > actual_values = read_values_from_tape(output_tape);

        BOOST_TEST(actual_values == expected_values);
    }
    std::filesystem::remove_all(get_test_dir());
}

BOOST_AUTO_TEST_CASE(tape_sorter_sorts_negative_and_duplicate_values)
{
    std::filesystem::remove_all(get_test_dir());
    {
        const std::vector< std::int32_t > input_values{100, -5, 0, 42, 42, -100, 7, 3, 1};
        const std::vector< std::int32_t > expected_values{-100, -5, 0, 1, 3, 7, 42, 42, 100};

        const TapeConfig config = get_test_config(12);

        FileTape input_tape(get_file_path("input2.bin").string(), input_values.size(), config);
        FileTape output_tape(get_file_path("output2.bin").string(), input_values.size(), config);

        write_values_to_tape(input_tape, input_values);

        TapeSorter sorter(config);
        sorter.sort(input_tape, output_tape);

        const std::vector< std::int32_t > actual_values = read_values_from_tape(output_tape);

        BOOST_TEST(actual_values == expected_values);
    }
    std::filesystem::remove_all(get_test_dir());
}

BOOST_AUTO_TEST_CASE(tape_sorter_throws_if_memory_limit_is_too_small)
{
    std::filesystem::remove_all(get_test_dir());
    {
        const std::vector< std::int32_t > input_values{3, 1, 2};

        const TapeConfig config = get_test_config(2);

        FileTape input_tape(get_file_path("input_small_memory.bin").string(), input_values.size(), config);
        FileTape output_tape(get_file_path("output_small_memory.bin").string(), input_values.size(), config);

        write_values_to_tape(input_tape, input_values);

        TapeSorter sorter(config);

        BOOST_CHECK_THROW(sorter.sort(input_tape, output_tape), std::runtime_error);
    }
    std::filesystem::remove_all(get_test_dir());
}
