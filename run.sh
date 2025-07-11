#!/usr/bin/env bash
set -euo pipefail

mkdir -p ./build/ && cd ./build/

cmake .. && make -j$(nproc)

printf "\n\n"
./McpingerCpp "$@"

