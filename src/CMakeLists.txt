include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(keyboard SHARED keyboard.cpp)
target_link_libraries(
      keyboard PRIVATE CONAN_PKG::sfml project_warnings project_options)

add_library(chip8 SHARED chip8.cpp)
target_link_libraries(
      chip8 PUBLIC keyboard PRIVATE CONAN_PKG::fmt CONAN_PKG::sfml project_warnings project_options)

add_executable(main_process main.cpp)
target_link_libraries(
      main_process PRIVATE chip8 keyboard CONAN_PKG::boost CONAN_PKG::fmt CONAN_PKG::argparse CONAN_PKG::imgui-sfml project_warnings project_options)

set_target_properties(chip8 main_process PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)



