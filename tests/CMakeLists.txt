include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(hamarc_tests hamming_test.cpp
)

target_link_libraries(
        hamarc_tests
        archiver
        GTest::gtest_main
)

target_include_directories(hamarc_tests PUBLIC ${PROJECT_SOURCE_DIR})

include(GoogleTest)

add_subdirectory(files)
add_custom_target(copy_test_files ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/files
        ${CMAKE_CURRENT_BINARY_DIR}/files
        COMMENT "Copying resources into binary directory")
add_dependencies(hamarc_tests copy_test_files)

gtest_discover_tests(hamarc_tests)
