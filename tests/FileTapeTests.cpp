#include "FileTape.h"

#include <boost/test/unit_test.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace
{

    std::filesystem::path get_test_dir()
    {
        return "tmp/tests_file_tape";
    }

    TapeConfig get_test_config()
    {
        return TapeConfig{0, 0, 0, 0, 1024};
    }

    std::filesystem::path get_tape_path(const std::string& file_name)
    {
        std::filesystem::create_directories(get_test_dir());
        return get_test_dir() / file_name;
    }

    void create_invalid_binary_file(const std::filesystem::path& file_path)
    {
        std::ofstream file(file_path, std::ios::binary | std::ios::trunc);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to create invalid binary test file");
        }

        const char invalid_bytes[3]{1, 2, 3};
        file.write(invalid_bytes, sizeof(invalid_bytes));
    }

}

BOOST_AUTO_TEST_CASE(file_tape_creates_tape_with_requested_size)
{
    std::filesystem::remove_all(get_test_dir());
    {
        FileTape tape(get_tape_path("created_tape.bin").string(), 5, get_test_config());

        BOOST_TEST(tape.size() == 5);
        BOOST_TEST(tape.position() == 0);
    }
    std::filesystem::remove_all(get_test_dir());
}

BOOST_AUTO_TEST_CASE(file_tape_writes_and_reads_values)
{
    std::filesystem::remove_all(get_test_dir());
    {
        FileTape tape(get_tape_path("values_tape.bin").string(), 3, get_test_config());

        tape.write(10);
        BOOST_TEST(tape.move_right());

        tape.write(-5);
        BOOST_TEST(tape.move_right());

        tape.write(42);

        tape.rewind();

        BOOST_TEST(tape.read() == 10);
        BOOST_TEST(tape.move_right());

        BOOST_TEST(tape.read() == -5);
        BOOST_TEST(tape.move_right());

        BOOST_TEST(tape.read() == 42);
    }
    std::filesystem::remove_all(get_test_dir());
}

BOOST_AUTO_TEST_CASE(file_tape_movement_boundaries)
{
    std::filesystem::remove_all(get_test_dir());
    {
        FileTape tape(get_tape_path("movement_tape.bin").string(), 2, get_test_config());

        BOOST_TEST(tape.position() == 0);
        BOOST_TEST(!tape.move_left());

        BOOST_TEST(tape.move_right());
        BOOST_TEST(tape.position() == 1);

        BOOST_TEST(!tape.move_right());

        BOOST_TEST(tape.move_left());
        BOOST_TEST(tape.position() == 0);
    }
    std::filesystem::remove_all(get_test_dir());
}

BOOST_AUTO_TEST_CASE(file_tape_reopens_existing_tape_and_reads_saved_values)
{
    std::filesystem::remove_all(get_test_dir());
    {
        const std::filesystem::path file_path = get_tape_path("reopen_tape.bin");

        {
            FileTape tape(file_path.string(), 2, get_test_config());

            tape.write(7);
            BOOST_TEST(tape.move_right());
            tape.write(99);
        }

        FileTape reopened_tape(file_path.string(), get_test_config());

        BOOST_TEST(reopened_tape.size() == 2);
        BOOST_TEST(reopened_tape.read() == 7);

        BOOST_TEST(reopened_tape.move_right());
        BOOST_TEST(reopened_tape.read() == 99);
    }
    std::filesystem::remove_all(get_test_dir());
}

BOOST_AUTO_TEST_CASE(file_tape_throws_if_binary_file_size_is_invalid)
{
    std::filesystem::remove_all(get_test_dir());
    {
        const std::filesystem::path file_path = get_tape_path("invalid_size.bin");

        create_invalid_binary_file(file_path);

        BOOST_CHECK_THROW(
            FileTape(file_path.string(), get_test_config()),
            std::runtime_error
        );
    }
    std::filesystem::remove_all(get_test_dir());
}
