#!/bin/bash
set -e

./scripts/build.sh 
echo "Running askFM..."
./askFM
