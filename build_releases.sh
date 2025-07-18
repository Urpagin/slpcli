#!/usr/bin/env bash

# Author: ChatGPTo3. This file was not made by a human.

# build_release.sh – produce {libc,musl} artefacts for slpcli
set -euo pipefail

################################################################################
# 0.  SETTINGS – tweak once
################################################################################
PROJECT=slpcli
TARGET=slpcli                     # add_executable() name
BUILD_ROOT=build_manual_rel       # all build dirs go here
DEST_DIR=dist

################################################################################
# 1.  COMMON INFO
################################################################################
ARCH=$(uname -m)
OS=$(uname -s | tr '[:upper:]' '[:lower:]')

# Helper: <build-dir> → version string read from configure step
get_version () { cat "$1/version.txt"; }

################################################################################
# 2.  LIBC (dynamic glibc) build
################################################################################
build_libc () {
  local build_dir="${BUILD_ROOT}/libc"
  echo "▶ [libc] Configuring…"
  cmake -S . -B "$build_dir" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_FLAGS_RELEASE="-O2 -DNDEBUG -flto"

  echo "▶ [libc] Building…"
  cmake --build "$build_dir" --target "$TARGET" --parallel

  local ver=$(get_version "$build_dir")
  local fname="${PROJECT}-${ver}-${ARCH}-${OS}-libc"

  echo "▶ [libc] Packaging ${fname}…"
  mkdir -p "$DEST_DIR"
  cp  "$build_dir/$TARGET" "$DEST_DIR/$TARGET"
  strip "$DEST_DIR/$TARGET"
  mv "$DEST_DIR/$TARGET" "$DEST_DIR/$fname"
  echo "✔ libc artefact: $DEST_DIR/$fname"
}

################################################################################
# 3.  MUSL (fully static) build inside Alpine container
################################################################################
build_musl () {
  local build_dir="${BUILD_ROOT}/musl"
  echo "▶ [musl] Building inside Alpine…"
  sudo docker run --rm -v "$PWD":/src -w /src alpine:edge sh -exc "
    apk add --no-cache build-base cmake ninja git musl-dev linux-headers
    cmake -B $build_dir -G Ninja \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_EXE_LINKER_FLAGS='-static -s'
    cmake --build $build_dir --target $TARGET --parallel
  "

  local ver=$(get_version "$build_dir")
  local fname="${PROJECT}-${ver}-${ARCH}-${OS}-musl"

  echo "▶ [musl] Packaging ${fname}…"
  mkdir -p "$DEST_DIR"
  cp  "$build_dir/$TARGET" "$DEST_DIR/$TARGET"
  strip "$DEST_DIR/$TARGET"
  mv "$DEST_DIR/$TARGET" "$DEST_DIR/$fname"
  echo "✔ musl artefact: $DEST_DIR/$fname"
}

################################################################################
# 4.  ENTRY‑POINT
################################################################################
case "${1-}" in
  libc) build_libc ;;
  musl) build_musl ;;
  ""  ) build_libc; build_musl ;;   # default: build both
  *   ) echo "Usage: $0 [libc|musl]" >&2; exit 1 ;;
esac
