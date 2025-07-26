#!/usr/bin/env bash
set -euo pipefail

# Clean & re-configure
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"

echo "▶ Cleaning previous build …"
rm -rf  "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd       "$BUILD_DIR"

cmake -G Xcode \
  -DOBS_STUDIO_PATH="$HOME/code/obs-sdk" \
  -DCMAKE_OSX_SYSROOT="$(xcrun --sdk macosx --show-sdk-path)" \
  -DOBS_MACOS_SDK_VERSION="$(xcrun --sdk macosx --show-sdk-version)" \
  -DENABLE_QT=ON \
  -DENABLE_FRONTEND_API=ON \
  -DCMAKE_OSX_ARCHITECTURES="arm64" \
  ..\

echo "✅  Plugin built successfully"