cmake_minimum_required(VERSION 3.15)

project(askFM
    VERSION 1.0.0
    DESCRIPTION "ask.fm clone"
    LANGUAGES CXX
)

# ============================================================================
# Configuration Options
# ============================================================================
option(BUILD_SHARED_LIBS "Build shared library instead of static" OFF)
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

# ============================================================================
# Source Files
# ============================================================================
file(GLOB SUB_SOURCES   "${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.cpp")
file(GLOB ROOT_SOURCES  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB ROOT_HEADERS  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")
file(GLOB SUB_HEADERS   "${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.hpp")

# CHANGE 1: Separate main.cpp from the rest of the sources
#           so the test target can reuse all logic without main()
list(FILTER ROOT_SOURCES EXCLUDE REGEX ".*main\\.cpp$")
file(GLOB MAIN_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

set(LIB_SOURCES ${ROOT_SOURCES} ${SUB_SOURCES})

# ============================================================================
# CHANGE 2: Core logic built as a static library
#           Both the main executable and tests link against it
# ============================================================================
add_library(${PROJECT_NAME}_lib STATIC ${LIB_SOURCES} ${ROOT_HEADERS} ${SUB_HEADERS})

target_include_directories(${PROJECT_NAME}_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(${PROJECT_NAME}_lib PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# ============================================================================
# Main Executable
# ============================================================================
add_executable(${PROJECT_NAME} ${MAIN_SOURCE})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)

# ============================================================================
# CHANGE 3: GTest fetched automatically — no manual install needed
# ============================================================================
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ============================================================================
# CHANGE 4: Test executable links against the lib, not the main executable
# ============================================================================
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")

add_executable(askFM_tests ${TEST_SOURCES})

target_link_libraries(askFM_tests PRIVATE
    ${PROJECT_NAME}_lib
    GTest::gtest_main
)

target_include_directories(askFM_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# CHANGE 5: Registers tests with CTest so you can run: ctest --test-dir build
include(GoogleTest)
gtest_discover_tests(askFM_tests)

option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # library
        target_compile_options(${PROJECT_NAME}_lib PRIVATE --coverage -O0 -g)
        target_link_options(${PROJECT_NAME}_lib PRIVATE --coverage)

        # main executable  ← this was missing
        target_compile_options(${PROJECT_NAME} PRIVATE --coverage -O0 -g)
        target_link_options(${PROJECT_NAME} PRIVATE --coverage)

        # tests
        target_compile_options(askFM_tests PRIVATE --coverage -O0 -g)
        target_link_options(askFM_tests PRIVATE --coverage)
    else()
        message(WARNING "Coverage only supported with GCC or Clang")
    endif()
endif()