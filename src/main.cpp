#include "FileTape.h"
#include "TapeSorter.h"
#include "ConfigReader.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

int main()
{
    try {
        const TapeConfig config = ConfigReader::read_config("config.txt");

        const std::vector<std::int32_t> input_values{
            8, 3, 5, 1, 9, 7, 4, 2
        };

        FileTape input_tape("tmp/manual_input.bin", input_values.size(), config);

        for (std::size_t i = 0; i < input_values.size(); ++i) {
            input_tape.write(input_values[i]);

            if (i + 1 < input_values.size()) {
                if (!input_tape.move_right()) {
                    throw std::runtime_error("Failed to move input tape while filling test data");
                }
            }
        }

        FileTape output_tape("tmp/manual_output.bin", input_values.size(), config);

        TapeSorter sorter(config);
        sorter.sort(input_tape, output_tape);

        output_tape.rewind();

        std::cout << "Sorted values: ";

        for (std::size_t i = 0; i < output_tape.size(); ++i) {
            std::cout << output_tape.read();

            if (i + 1 < output_tape.size()) {
                std::cout << ' ';

                if (!output_tape.move_right()) {
                    throw std::runtime_error("Failed to move output tape while printing result");
                }
            }
        }

        std::cout << '\n';
    } catch (const std::exception& exception) {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}
