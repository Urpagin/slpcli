#!/usr/bin/env bash

# release.sh – build an optimized, stripped Release binary
set -euo pipefail

ARCH=$(uname -m)
OS=$(uname -s | tr '[:upper:]' '[:lower:]')
VERSION="x.y.z"
FILENAME="slpcli-${VERSION}-${ARCH}-${OS}"

BUILD_DIR=build/release      # isolated Release build tree
TARGET="slpcli"
DEST_DIR=dist                # final location for the binary

echo "▶ Configuring CMake (Release)…"
cmake -S . -B "$BUILD_DIR" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS_RELEASE="-O2 -DNDEBUG -flto" \
      -G Ninja

echo "▶ Building…"
cmake --build "$BUILD_DIR" --target "$TARGET" --parallel

echo "▶ Collecting and stripping binary…"
mkdir -p "$DEST_DIR"
cp "$BUILD_DIR/$TARGET" "$DEST_DIR/"
strip "$DEST_DIR/$TARGET"
mv "$DEST_DIR/$TARGET" "$DEST_DIR/$FILENAME"

echo "✔ Finished: $DEST_DIR/$FILENAME"

