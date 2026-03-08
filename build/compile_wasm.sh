#!/bin/bash

# C99 → WebAssembly Build Script
# Uses Emscripten to compile C99 DSP library to WebAssembly

set -e

echo "🔧 C99 → WebAssembly Build Script"

# Path configuration
SRC_DIR="src/c"
WASM_DIR="src/wasm"
BUILD_DIR="build"
EMSDK_DIR="/var/services/homes/seo/.openclaw/workspace/emsdk"

# Check if Emscripten is installed
if [ ! -d "$EMSDK_DIR" ]; then
    echo "❌ Emscripten is not installed."
    echo "Install method:"
    echo "  git clone https://github.com/emscripten-core/emsdk.git $EMSDK_DIR"
    echo "  cd $EMSDK_DIR"
    echo "  ./emsdk install latest"
    echo "  ./emsdk activate latest"
    exit 1
fi

# Activate Emscripten
source "$EMSDK_DIR/emsdk_env.sh"

echo "✅ Emscripten path configured"
echo "   EMSDK: $EMSDK_DIR"
echo "   EMCC: $(which emcc)"

# WebAssembly compilation flags
EMCC_FLAGS=(
    "-s USE_SDL=0"
    "-s USE_SDL_AUDIO=0"
    "-O3"  # Optimization level 3 (highest)
    "-s ALLOW_MEMORY_GROWTH=1"
    "-s MODULARIZE=1"
    "-s EXPORT_NAME=\"FFTModule\""
    "-s EXPORTED_FUNCTIONS='[\"_malloc\", \"_free\", \"_fft_radix2\"]'"
    "-s WASM=1"
    "-msimd128"  # Enable 128-bit SIMD (SIMD128)
    "--memory-init 8192"  # Initialize with 8KB memory
)

# C99 source files
C_SOURCES=(
    "$SRC_DIR/fft/fft_radix2.c"
    "$SRC_DIR/filters/biquad_fir.c"
)

echo ""
echo "🚀 WebAssembly Compilation Started..."
echo "   Source files: ${C_SOURCES[@]}"

# Compile C99 → WebAssembly
emcc "${EMCC_FLAGS[@]}" \
    -I "$SRC_DIR/.." \
    "${C_SOURCES[@]}" \
    -o "$BUILD_DIR/fft.js"

if [ $? -eq 0 ]; then
    echo "✅ Compilation Successful!"
    echo "   Output: $BUILD_DIR/fft.js"
    
    # WebAssembly file size
    WASM_SIZE=$(stat -f%s "$BUILD_DIR/fft.wasm" 2>/dev/null || echo "N/A")
    echo "   Size: ${WASM_SIZE} bytes"
else
    echo "❌ Compilation Failed!"
    exit 1
fi

echo ""
echo "📊 Additional Optimization..."

# Binaryen optimization (optional)
if command -v wasm-opt >/dev/null 2>&1; then
    echo "🔧 Binaryen Optimization..."
    wasm-opt -O3 "$BUILD_DIR/fft.wasm" -o "$BUILD_DIR/fft_optimized.wasm"
    
    if [ $? -eq 0 ]; then
        echo "✅ Binaryen Optimization Complete!"
        echo "   Output: $BUILD_DIR/fft_optimized.wasm"
    else
        echo "⚠️  Binaryen Optimization Failed (will be ignored)"
    fi
else
    echo "⚠️  wasm-opt not installed (optimization skipped)"
fi

echo ""
echo "📋 Build Files:"
echo "   JavaScript: $BUILD_DIR/fft.js"
echo "   WebAssembly: $BUILD_DIR/fft.wasm"
echo "   Binaryen: $BUILD_DIR/fft_optimized.wasm (optional)"

echo ""
echo "🎯 Testing on Web:"
echo "   python3 -m http.server 8080"
echo "   Open in browser: file://$PWD/web/benchmarks.html"

echo ""
echo "✅ Build Complete!"
