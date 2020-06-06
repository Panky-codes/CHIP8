# Basic CMake project
cmake_minimum_required(VERSION 3.5.1)
project(CHIP8)
    
#Change to RELEASE when the project is done
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#Add all the project options necessary for targets
add_library(project_options INTERFACE)

target_compile_features(project_options INTERFACE cxx_std_17)

add_library(project_warnings INTERFACE)
# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# Set up some extra Conan dependencies based on our needs
# before loading Conan
set(CONAN_EXTRA_REQUIRES "")
set(CONAN_EXTRA_OPTIONS "")

include(cmake/Conan.cmake)
run_conan()

# ------------------------------------------------------------------------------
# Google Sanitizers
# ------------------------------------------------------------------------------
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)
# ------------------------------------------------------------------------------
# Valgrind
# ------------------------------------------------------------------------------

set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --leak-check=full")
set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --track-fds=yes")
set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --trace-children=yes")
set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1")
# ------------------------------------------------------------------------------


# ------------------------------------------------------------------------------
# CPPCHECK
# ------------------------------------------------------------------------------

option(ENABLE_CPPCHECK "Enable static analysis with cppcheck" OFF)

if(ENABLE_CPPCHECK)
  find_program(CPPCHECK cppcheck)
  if(CPPCHECK)
    set(CMAKE_CXX_CPPCHECK ${CPPCHECK} --suppress=missingInclude --suppress=unusedFunction --enable=all --inconclusive )
  else()
    message(SEND_ERROR "cppcheck requested but executable not found")
  endif()
endif()
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# CPPCHECK
# ------------------------------------------------------------------------------

option(ENABLE_INCLUDE_WHAT_YOU_USE "Enable static analysis with include-what-you-use" OFF)
if(ENABLE_INCLUDE_WHAT_YOU_USE)
  find_program(INCLUDE_WHAT_YOU_USE include-what-you-use)
  if(INCLUDE_WHAT_YOU_USE)
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${INCLUDE_WHAT_YOU_USE})
    message("HELO!!!!!" ${INCLUDE_WHAT_YOU_USE})
  else()
    message(SEND_ERROR "include-what-you-use requested but executable not found")
  endif()
endif()
# ------------------------------------------------------------------------------
if(ENABLE_TESTING)
  enable_testing()
  include(CTest)
  include(Catch)
  message(
    "Building Tests"
  )
  add_subdirectory(tests)
endif()

ADD_SUBDIRECTORY(src)
