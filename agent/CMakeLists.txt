cmake_minimum_required(VERSION 3.20)
project(factorypulse_agent LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(FP_BUILD_TESTS "Build the unit tests" ON)

include(FetchContent)
find_package(Threads REQUIRED)

# --- Dependencies: use installed packages when available, otherwise download them ---

find_package(nlohmann_json 3.10 QUIET)
if(NOT nlohmann_json_FOUND)
  FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
  FetchContent_MakeAvailable(json)
endif()

find_package(httplib CONFIG QUIET)
if(NOT TARGET httplib::httplib)
  find_path(HTTPLIB_INCLUDE_DIR httplib.h)
  # Some Linux distributions ship cpp-httplib precompiled (libcpp-httplib.so) instead of header-only.
  find_library(HTTPLIB_LIBRARY NAMES cpp-httplib)
  if(HTTPLIB_INCLUDE_DIR)
    add_library(httplib_header INTERFACE)
    target_include_directories(httplib_header INTERFACE ${HTTPLIB_INCLUDE_DIR})
    if(HTTPLIB_LIBRARY)
      target_link_libraries(httplib_header INTERFACE ${HTTPLIB_LIBRARY})
    endif()
    add_library(httplib::httplib ALIAS httplib_header)
  else()
    FetchContent_Declare(httplib
      GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
      GIT_TAG v0.18.3)
    FetchContent_MakeAvailable(httplib)
  endif()
endif()

# --- Core library (everything except main), shared by the program and the tests ---

add_library(fp_core
  src/reading.cpp
  src/sensor.cpp
  src/machine.cpp
  src/publisher.cpp
  src/agent.cpp)
target_include_directories(fp_core PUBLIC include)
target_link_libraries(fp_core PUBLIC nlohmann_json::nlohmann_json PRIVATE httplib::httplib Threads::Threads)
if(WIN32)
  target_link_libraries(fp_core PRIVATE ws2_32)
endif()
if(MSVC)
  target_compile_options(fp_core PRIVATE /W4)
else()
  target_compile_options(fp_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(fp_agent src/main.cpp)
target_link_libraries(fp_agent PRIVATE fp_core)

# --- Tests ---

if(FP_BUILD_TESTS)
  enable_testing()
  find_package(GTest QUIET)
  if(NOT GTest_FOUND)
    FetchContent_Declare(googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  endif()

  add_executable(fp_tests
    tests/sensor_test.cpp
    tests/machine_test.cpp
    tests/agent_test.cpp)
  target_link_libraries(fp_tests PRIVATE fp_core GTest::gtest_main)

  include(GoogleTest)
  gtest_discover_tests(fp_tests)
endif()
