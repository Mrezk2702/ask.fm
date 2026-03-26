#!/bin/bash
set -e  # stop on first error

echo "Cleaning build artifacts..."

rm -rf build/
rm -rf debug/
rm -rf coverage_report/
rm -f  coverage.info
rm -f  askFM
rm -f  askFM_tests

echo "Done."