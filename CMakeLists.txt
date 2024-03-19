cmake_minimum_required(VERSION 3.12)

project(
        labwork6
        VERSION 1.0
        DESCRIPTION "C++ Year 2023 labwork 6"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)


add_subdirectory(lib)
add_subdirectory(argparser)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(bin)
add_subdirectory(files)

#file(GLOB files "files/*")
#foreach(file IN LISTS file)
#    configure_file(${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} @ONLY)
#endforeach()
add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/files
        ${PROJECT_BINARY_DIR}/files
        COMMENT "Copying resources into binary directory")

add_executable(labwork6 src/hamarc.cpp)
target_link_libraries(labwork6 argparser archiver)
add_dependencies(labwork6 copy_resources)

include_directories(lib argparser)