include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(catch_main STATIC tests-main.cpp)
target_link_libraries(catch_main PUBLIC CONAN_PKG::catch2)

add_executable(test_chip8_bin tests-chip8.cpp)
target_link_libraries(test_chip8_bin PUBLIC chip8 project_options catch_main CONAN_PKG::fmt)
target_compile_options(test_chip8_bin PUBLIC -Wall -Wextra -pedantic-errors -Wconversion -Wsign-conversion)
catch_discover_tests(test_chip8_bin)


set_target_properties(test_chip8_bin catch_main PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
)
