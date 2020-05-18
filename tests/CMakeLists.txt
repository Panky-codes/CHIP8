include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(test_chip8_bin tests-chip8.cpp)
target_link_libraries(test_chip8_bin PUBLIC chip8 CONAN_PKG::catch2 CONAN_PKG::fmt)
target_compile_options(test_chip8_bin PUBLIC -Werror -Wall -Wextra -pedantic-errors -Wconversion -Wsign-conversion)
add_test(test_chip8 ${CMAKE_BINARY_DIR}/bin/test_chip8_bin)


set_target_properties(test_chip8_bin PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
)
