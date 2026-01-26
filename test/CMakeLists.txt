cmake_minimum_required(VERSION 3.10)
project(KegmonStateMachineTests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add source includes
include_directories(${CMAKE_SOURCE_DIR}/../src)
include_directories(${CMAKE_SOURCE_DIR}/../)

# Integration test: Validates the ACTUAL POURING exit logic from changedetection.cpp
# This test extracts and validates the core state machine logic WITHOUT Arduino dependencies
add_executable(changedetection_integration_test 
  changedetection_integration_test.cpp
)

# Link against gtest only (logic extracted, no production code dependencies)
target_link_libraries(changedetection_integration_test 
  gtest
  gtest_main
)

# Enable testing
enable_testing()
add_test(NAME ChangeDetectionIntegration COMMAND changedetection_integration_test)
