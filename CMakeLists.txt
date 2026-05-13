cmake_minimum_required(VERSION 3.16)

project(tape_sorter)

set(CMAKE_CXX_STANDARD 20)

add_executable(tape_sorter
    src/main.cpp
    src/ConfigReader.cpp
    src/FileTape.cpp
    src/TapeSorter.cpp
    src/TapeFileConverter.cpp
)

target_include_directories(tape_sorter PRIVATE include)

include(CTest)

if(BUILD_TESTING)
    add_executable(tape_sorter_tests
        tests/TestMain.cpp
        tests/ConfigReaderTests.cpp
        tests/FileTapeTests.cpp
        tests/TapeSorterTests.cpp
        src/ConfigReader.cpp
        src/FileTape.cpp
        src/TapeSorter.cpp
    )

    target_include_directories(tape_sorter_tests PRIVATE include)

    add_test(
        NAME tape_sorter_tests
        COMMAND tape_sorter_tests
    )
endif()
