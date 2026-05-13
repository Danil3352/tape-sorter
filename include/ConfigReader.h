#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <string>

#include "TapeConfig.h"

class ConfigReader
{
public:
    static TapeConfig read_config(const std::string& file_path);
};

#endif
