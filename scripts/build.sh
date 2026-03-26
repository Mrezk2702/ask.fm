#!/bin/bash

#./scripts/build.sh         # Debug as default
# ./scripts/build.sh Release # Release

set -e

BUILD_TYPE=${1:-Debug}   # default to Debug, pass "Release" as arg

echo "Building in $BUILD_TYPE mode..."
cmake -S . -B build -DCMAKE_BUILD_TYPE=$BUILD_TYPE
cmake --build build --parallel$(nproc)
echo "Done. Executable at ./askFM"