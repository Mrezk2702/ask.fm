#!/bin/bash
set -e

echo "Cleaning..."
./scripts/clean.sh

echo "Building..."
./scripts/build.sh $1