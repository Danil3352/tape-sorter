#include "ConfigReader.h"
#include "FileTape.h"
#include "TapeFileConverter.h"
#include "TapeSorter.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: tape_sorter <input_file> <output_file>\n";
        return 1;
    }

    try {
        const std::string input_file_path = argv[1];
        const std::string output_file_path = argv[2];
        if (input_file_path == output_file_path) {
            throw std::runtime_error("Input and output file paths must be different");
        }
        const std::filesystem::path input_path(input_file_path);
        const std::filesystem::path output_path(output_file_path);

        const bool input_is_text = input_path.extension() == ".txt";
        const bool output_is_text = output_path.extension() == ".txt";

        std::filesystem::create_directories("tmp");

        const std::string binary_input_file_path = input_is_text ? "tmp/converted_input.bin" : input_file_path;

        const std::string binary_output_file_path = output_is_text ? "tmp/converted_output.bin" : output_file_path;

        if (input_is_text) {
            TapeFileConverter::text_to_binary(input_file_path, binary_input_file_path);
        }

        const TapeConfig config = ConfigReader::read_config("config.txt");

        {
            FileTape input_tape(binary_input_file_path, config);
            FileTape output_tape(binary_output_file_path, input_tape.size(), config);

            TapeSorter sorter(config);
            sorter.sort(input_tape, output_tape);
        }

        if (output_is_text) {
            TapeFileConverter::binary_to_text(binary_output_file_path, output_file_path);
        }

        if (input_is_text) {
            std::filesystem::remove(binary_input_file_path);
        }

        if (output_is_text) {
            std::filesystem::remove(binary_output_file_path);
        }

        std::cout << "Sorted successfully.\n";
    } catch (const std::exception& exception) {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}
