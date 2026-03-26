#!/bin/bash

# ./scripts/test.sh                                        # all tests
# ./scripts/test.sh --gtest_filter=AuthTest.*              # one suite
# ./scripts/test.sh --gtest_filter=AuthTest.Login_*        # pattern
set -e

echo "Building tests..."
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target askFM_tests --parallel $(nproc)

echo "Running tests..."
./build/askFM_tests --gtest_color=yes $@