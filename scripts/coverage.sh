#!/bin/bash
set -e

echo "Building with coverage..."
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
cmake --build build --parallel $(nproc)

echo "Running tests..."
./askFM_tests

echo "Collecting coverage data..."
lcov --capture --directory build --output-file coverage.info \
     --ignore-errors mismatch

lcov --remove coverage.info \
    '/usr/*' \
    '*/googletest/*' \
    '*/tests/*' \
    --output-file coverage.info \
    --ignore-errors mismatch

genhtml coverage.info --output-directory coverage_report

echo "Coverage report at coverage_report/index.html"
xdg-open coverage_report/index.html 2>/dev/null || true