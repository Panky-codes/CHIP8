add_executable(main_process main.cpp chip8.cpp)

target_link_libraries(
      main_process PRIVATE CONAN_PKG::fmt CONAN_PKG::spdlog)

target_include_directories(main_process PRIVATE ${CMAKE_SOURCE_DIR}/include)

set_target_properties(main_process PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    COMPILE_FLAGS "-Wall -Wextra -Wpedantic -Werror"
)


