#!/usr/bin/env bash
set -euo pipefail

mkdir -p ./build/ && cd ./build/

cmake .. \
  -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wnull-dereference -Wdouble-promotion -Wformat=2 -Werror"

make -j"$(nproc)"

printf "\n\n"
./slpcli "$@"
