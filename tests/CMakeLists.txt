
add_executable(chip8 tests-vector.cpp)
target_link_libraries(chip8 PUBLIC CONAN_PKG::catch2)
add_test(test_chip8 chip8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
target_include_directories(chip8 PUBLIC ${CONAN_INCLUDE_DIRS})
target_include_directories(chip8 PUBLIC ${CMAKE_BINARY_DIR}/include)
set_target_properties(chip8 PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    COMPILE_FLAGS "-Wall -g -Wextra -Wpedantic -Werror"
)
