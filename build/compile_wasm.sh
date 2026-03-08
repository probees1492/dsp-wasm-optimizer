#!/bin/bash

# C99 → WebAssembly 컴파일 스크립트
# Emscripten 사용

set -e

echo "🔧 C99 → WebAssembly 컴파일 도구"

# 경로 설정
SRC_DIR="src/c"
WASM_DIR="src/wasm"
BUILD_DIR="build"
EMSDK_DIR="/var/services/homes/seo/.openclaw/workspace/emsdk"

# Emscripten 경로 체크
if [ ! -d "$EMSDK_DIR" ]; then
    echo "❌ Emscripten이 설치되어 있지 않습니다."
    echo "설치 방법:"
    echo "  git clone https://github.com/emscripten-core/emsdk.git $EMSDK_DIR"
    echo "  cd $EMSDK_DIR"
    echo "  ./emsdk install latest"
    echo "  ./emsdk activate latest"
    exit 1
fi

# 활성화
source "$EMSDK_DIR/emsdk_env.sh"

echo "✅ Emscripten 경로 설정 완료"
echo "   EMSDK: $EMSDK_DIR"
echo "   EMCC: $(which emcc)"

# WebAssembly 설정
EMCC_FLAGS=(
    "-s USE_SDL=0"
    "-s USE_SDL_AUDIO=0"
    "-O3"  # 최적화 레벨
    "-s ALLOW_MEMORY_GROWTH=1"
    "-s MODULARIZE=1"
    "-s EXPORT_NAME=\"FFTModule\""
    "-s EXPORTED_FUNCTIONS='[\"_malloc\", \"_free\", \"_fft_radix2\"]'"
    "-s WASM=1"
    "-msimd128"
    "--memory-init 8192"
)

# C99 소스 파일들
C_SOURCES=(
    "$SRC_DIR/fft/fft_radix2.c"
    "$SRC_DIR/filters/biquad_fir.c"
)

echo ""
echo "🚀 WebAssembly 컴파일 시작..."
echo "   소스 파일: ${C_SOURCES[@]}"

# C99 → WebAssembly 컴파일
emcc "${EMCC_FLAGS[@]}" \
    -I "$SRC_DIR/.." \
    "${C_SOURCES[@]}" \
    -o "$BUILD_DIR/fft.js"

if [ $? -eq 0 ]; then
    echo "✅ 컴파일 성공!"
    echo "   출력: $BUILD_DIR/fft.js"
    
    # WebAssembly 파일 크기
    WASM_SIZE=$(stat -f%s "$BUILD_DIR/fft.wasm" 2>/dev/null || echo "N/A")
    echo "   크기: ${WASM_SIZE} bytes"
else
    echo "❌ 컴파일 실패!"
    exit 1
fi

echo ""
echo "📊 추가 최적화..."

# Binaryen으로 최적화 (선택 사항)
if command -v wasm-opt >/dev/null 2>&1; then
    echo "🔧 Binaryen 최적화 중..."
    wasm-opt -O3 "$BUILD_DIR/fft.wasm" -o "$BUILD_DIR/fft_optimized.wasm"
    
    if [ $? -eq 0 ]; then
        echo "✅ Binaryen 최적화 완료!"
        echo "   출력: $BUILD_DIR/fft_optimized.wasm"
    else
        echo "⚠️  Binaryen 최적화 실패 (무시됨)"
    fi
else
    echo "⚠️  wasm-opt가 설치되어 있지 않습니다 (무시됨)"
fi

echo ""
echo "📋 빌드 파일 정보:"
echo "   JavaScript: $BUILD_DIR/fft.js"
echo "   WebAssembly: $BUILD_DIR/fft.wasm"
echo "   Binaryen: $BUILD_DIR/fft_optimized.wasm (선택사항)"

echo ""
echo "🎯 웹에서 테스트 방법:"
echo "   python3 -m http.server 8080"
echo "   브라우저에서 file://$PWD/web/benchmarks.html 열기"

echo ""
echo "✅ 컴파일 완료!"
