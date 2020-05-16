include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(chip8 SHARED chip8.cpp)
target_link_libraries(
      chip8 PUBLIC CONAN_PKG::fmt CONAN_PKG::spdlog)
target_compile_options(chip8 PRIVATE -Werror -Wall -Wextra -pedantic-errors -Wconversion -Wsign-conversion)


add_executable(main_process main.cpp )
target_link_libraries(
      main_process PRIVATE chip8 CONAN_PKG::fmt CONAN_PKG::spdlog)
target_compile_options(main_process PRIVATE -Werror -Wall -Wextra -pedantic-errors -Wconversion -Wsign-conversion)

set_target_properties(chip8 main_process PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)


