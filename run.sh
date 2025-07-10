#!/usr/bin/env bash

mkdir -p ./build/

cd ./build/

cmake .. .

make -j$(nproc)

printf "\n\n"

./McpingerCpp "$@"

