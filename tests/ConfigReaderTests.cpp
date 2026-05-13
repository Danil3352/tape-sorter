#include "ConfigReader.h"

#include <boost/test/unit_test.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace
{

    std::filesystem::path get_test_dir()
    {
        return "tmp/tests_config_reader";
    }

    std::filesystem::path create_config_file(
        const std::string& file_name,
        const std::string& content
    )
    {
        std::filesystem::create_directories(get_test_dir());

        const std::filesystem::path file_path = get_test_dir() / file_name;

        std::ofstream file(file_path);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to create test config file");
        }

        file << content;

        return file_path;
    }

}

BOOST_AUTO_TEST_CASE(config_reader_reads_valid_config)
{
    std::filesystem::remove_all(get_test_dir());

    const std::filesystem::path config_path = create_config_file(
        "valid_config.txt",
        "read_delay_ms=1\n"
        "write_delay_ms=2\n"
        "move_delay_ms=3\n"
        "rewind_delay_ms=4\n"
        "m_limit_bytes=1024\n"
    );

    const TapeConfig config = ConfigReader::read_config(config_path.string());

    BOOST_TEST(config.read_delay_ms == 1);
    BOOST_TEST(config.write_delay_ms == 2);
    BOOST_TEST(config.move_delay_ms == 3);
    BOOST_TEST(config.rewind_delay_ms == 4);
    BOOST_TEST(config.m_limit_bytes == 1024);

    std::filesystem::remove_all(get_test_dir());
}

BOOST_AUTO_TEST_CASE(config_reader_throws_on_duplicate_key)
{
    std::filesystem::remove_all(get_test_dir());

    const std::filesystem::path config_path = create_config_file(
        "duplicate_key_config.txt",
        "read_delay_ms=1\n"
        "read_delay_ms=2\n"
        "move_delay_ms=3\n"
        "rewind_delay_ms=4\n"
        "m_limit_bytes=1024\n"
    );

    BOOST_CHECK_THROW(ConfigReader::read_config(config_path.string()), std::runtime_error);

    std::filesystem::remove_all(get_test_dir());
}

BOOST_AUTO_TEST_CASE(config_reader_throws_on_unknown_key)
{
    std::filesystem::remove_all(get_test_dir());

    const std::filesystem::path config_path = create_config_file(
        "unknown_key_config.txt",
        "read_delay_ms=1\n"
        "write_delay_ms=2\n"
        "move_delay_ms=3\n"
        "rewind_delay_ms=4\n"
        "unknown_key=1024\n"
    );

    BOOST_CHECK_THROW(ConfigReader::read_config(config_path.string()), std::runtime_error);

    std::filesystem::remove_all(get_test_dir());
}

BOOST_AUTO_TEST_CASE(config_reader_throws_on_invalid_numeric_value)
{
    std::filesystem::remove_all(get_test_dir());

    const std::filesystem::path config_path = create_config_file(
        "invalid_value_config.txt",
        "read_delay_ms=1\n"
        "write_delay_ms=2\n"
        "move_delay_ms=abc\n"
        "rewind_delay_ms=4\n"
        "m_limit_bytes=1024\n"
    );

    BOOST_CHECK_THROW(ConfigReader::read_config(config_path.string()), std::runtime_error);

    std::filesystem::remove_all(get_test_dir());
}
