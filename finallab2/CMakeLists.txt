cmake_minimum_required(VERSION 3.5)

project(SysYFCompiler)

set(CMAKE_CXX_STANDARD 17)

# required by clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# if not set, set CMAKE_BUILD_TYPE to Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -Wextra -Wno-unused -Wshadow -g -pedantic")
elseif(CMAKE_BUILD_TYPE STREQUAL "Asan")
    # see https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html#index-fsanitize_003daddress and https://github.com/google/sanitizers/wiki/AddressSanitizer for explanation of -fsanitize=address
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -Wextra -Wno-unused -Wshadow -g -pedantic -fsanitize=address -fno-omit-frame-pointer")
else()
    message(WARNING "Unknown build type: ${CMAKE_BUILD_TYPE}. Using default flags.")
endif()

# for ninja diagnostics color, see https://stackoverflow.com/questions/73349743/ninja-build-system-gcc-clang-doesnt-output-diagnostic-colors
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")

# include generated files in project environment
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/AST)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/ErrorReporter)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/Frontend)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/SysYFIR)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/SysYFIRBuilder)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/Log)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/Optimize)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/AST)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/ErrorReporter)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/Frontend)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/SysYFIR)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/SysYFIRBuilder)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/Log)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/Optimize)

# add_library(
#   BISON_FLEX_LIB STATIC
#   ${BISON_SysYFParser_OUTPUTS}
#   ${FLEX_SysYFScanner_OUTPUTS}
# )

add_executable(
  compiler
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(
  compiler
  SysYFPass
  SysYFIRBuilder
  IRLib
  Driver
  Checker
  ASTPrinter
  ErrReporter
  Log
  # BISON_FLEX_LIB
)

add_executable(
        test_logging
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Log/test_logging.cpp
)

target_link_libraries(
        test_logging
        Log
)
