#!/bin/bash

set -euo pipefail

if ! command -v cmake &> /dev/null; then
  echo "cmake could not be found, please install it"
  exit 1
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUTPUT_DIR="$ROOT_DIR/macos/rnllama-macos.framework"
BUILD_ROOT="$ROOT_DIR/.build-macos"
STAGING_DIR="$BUILD_ROOT/staging"

# Default to a universal (arm64 + x86_64) framework; override with
# RNLLAMA_MACOS_ARCHS=arm64 for a fast single-arch build.
ARCHS="${RNLLAMA_MACOS_ARCHS:-arm64;x86_64}"

cleanup() {
  rm -rf "$BUILD_ROOT"
}

trap cleanup EXIT

copy_headers() {
  local framework_path="$1"

  mkdir -p "$framework_path/Headers"
  cp "$ROOT_DIR"/cpp/*.h "$framework_path/Headers/"

  mkdir -p "$framework_path/Headers/jinja"
  cp "$ROOT_DIR"/cpp/common/jinja/*.h "$framework_path/Headers/jinja/"

  mkdir -p "$framework_path/Headers/nlohmann"
  cp "$ROOT_DIR"/cpp/nlohmann/*.hpp "$framework_path/Headers/nlohmann/"

  # Copy necessary common headers to Headers root (for includes without path prefix)
  cp "$ROOT_DIR"/cpp/common/chat.h "$framework_path/Headers/"
  cp "$ROOT_DIR"/cpp/common/common.h "$framework_path/Headers/"
  cp "$ROOT_DIR"/cpp/common/sampling.h "$framework_path/Headers/"
  cp "$ROOT_DIR"/cpp/common/speculative.h "$framework_path/Headers/"
  cp "$ROOT_DIR"/cpp/common/json.h "$framework_path/Headers/"
  cp "$ROOT_DIR"/cpp/common/json-schema-to-grammar.h "$framework_path/Headers/"
  cp "$ROOT_DIR"/cpp/common/peg-parser.h "$framework_path/Headers/"
}

assert_matching_dsym() {
  local framework_path="$1"
  local dsym_path="$2"
  local binary_uuids
  local dsym_uuids

  if [[ ! -d "$dsym_path" ]]; then
    echo "Missing dSYM bundle for $framework_path" >&2
    exit 1
  fi

  binary_uuids="$(dwarfdump --uuid "$framework_path/rnllama" | awk '{print $2 ":" $3}' | sort)"
  dsym_uuids="$(dwarfdump --uuid "$dsym_path/Contents/Resources/DWARF/rnllama" | awk '{print $2 ":" $3}' | sort)"

  if [[ "$binary_uuids" != "$dsym_uuids" ]]; then
    echo "dSYM UUID mismatch for $framework_path" >&2
    echo "Framework UUIDs:" >&2
    echo "$binary_uuids" >&2
    echo "dSYM UUIDs:" >&2
    echo "$dsym_uuids" >&2
    exit 1
  fi
}

t0=$(date +%s)

rm -rf "$OUTPUT_DIR"
mkdir -p "$STAGING_DIR"

(
  cd "$BUILD_ROOT"

  cmake "$ROOT_DIR/macos" \
    -GXcode \
    -DCMAKE_SYSTEM_NAME=Darwin \
    -DCMAKE_OSX_ARCHITECTURES="$ARCHS" \
    -DCMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=NO \
    -DCMAKE_INSTALL_PREFIX="$PWD/install"

  cmake --build . --config Release -j "$(sysctl -n hw.logicalcpu)"
)

FRAMEWORK_PATH="$BUILD_ROOT/Release/rnllama.framework"
DSYM_PATH="$BUILD_ROOT/Release/rnllama.framework.dSYM"

if [[ ! -d "$FRAMEWORK_PATH" ]]; then
  echo "Missing framework build output at $FRAMEWORK_PATH" >&2
  exit 1
fi

assert_matching_dsym "$FRAMEWORK_PATH" "$DSYM_PATH"

ditto "$FRAMEWORK_PATH" "$OUTPUT_DIR"
copy_headers "$OUTPUT_DIR"
ditto "$DSYM_PATH" "$OUTPUT_DIR.dSYM"

assert_matching_dsym "$OUTPUT_DIR" "$OUTPUT_DIR.dSYM"

t1=$(date +%s)
echo "Total time: $((t1 - t0)) seconds"
echo "Prebuilt macOS framework is in macos/rnllama-macos.framework"