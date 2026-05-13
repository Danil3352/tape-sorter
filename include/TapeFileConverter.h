#ifndef TAPE_FILE_CONVERTER_H
#define TAPE_FILE_CONVERTER_H

#include <string>

namespace TapeFileConverter
{

    void text_to_binary(const std::string& text_file_path, const std::string& binary_file_path);

    void binary_to_text(const std::string& binary_file_path,const std::string& text_file_path);

}

#endif
