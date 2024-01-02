cmake_minimum_required(VERSION 3.12)

project(
        hamming-archiver
        VERSION 1.0
        DESCRIPTION "C++ Year 2023 labwork 6"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)


add_subdirectory(lib)
add_subdirectory(argparser)
add_subdirectory(src)
add_subdirectory(tests)


add_executable(hamming-archiver src/hamarc.cpp)
target_link_libraries(hamming-archiver argparser archiver)

include_directories(lib argparser)
