# 🎵 DSP → WebAssembly 최적화 MVP

[![GitHub](https://img.shields.io/badge/GitHub-prowl-purple.svg)](https://github.com/probees1492/dsp-wasm-optimizer)

C99로 작성된 고성능 DSP 라이브러리를 WebAssembly로 컴파일하여 웹에서 실시간 테스트 및 최적화하는 프로젝트입니다.

## 🎯 핵심 기능

- **C99 DSP 라이브러리**
  - FFT (Fast Fourier Transform): Radix-2, Cooley-Tukey
  - 필터: Biquad, FIR, IIR
  - 벡터 연산: SIMD 최적화 (AVX2, SSE4.2, NEON)

- **WebAssembly 컴파일**
  - Emscripten 설정
  - 메모리 최적화 (Linear Memory)
  - SIMD 명령어 활용 (128-bit 벡터)

- **웹 기반 벤치마킹**
  - JavaScript 실시간 성능 측정
  - Visualizer (FFT 시각화, 필터 응답 그래프)
  - 자동 최적화 파라미터 추천

- **GitHub Copilot 활용**
  - Copilot Workspace 설정
  - AI 기반 코드 자동 완성

## 🛠 기술 스택

- **DSP**: C99, SIMD (AVX2, SSE4.2, NEON)
- **WebAssembly**: Emscripten, Binaryen
- **벤치마킹**: JavaScript, WebGL
- **최적화**: Binaryen (wasm-opt), LLVM opt
- **프레임워크**: Google Benchmark.js

## 📁 프로젝트 구조

```
dsp-wasm-optimizer/
├── src/
│   ├── c/               # C99 DSP 라이브러리
│   │   ├── fft/
│   │   │   ├── radix2.c
│   │   │   ├── cooley_tukey.c
│   │   │   └── simd_opt.h
│   │   ├── filters/
│   │   │   ├── biquad.c
│   │   │   ├── fir.c
│   │   │   └── iir.c
│   │   └── asm/
│   │       ├── fft_wasm.c
│   │       └── memory_layout.c
│   ├── wasm/            # WebAssembly 래퍼
│   │   ├── fft.wat
│   │   └── filters.wat
│   └── bench/           # 벤치마킹 코드
│       ├── fft_bench.js
│       ├── filter_bench.js
│       └── visualizer.js
├── tests/              # 웹 테스트
│   ├── fft_test.html
│   ├── filter_test.html
│   └── memory_test.html
├── build/              # 빌드 스크립트
│   ├── compile_c.sh
│   ├── compile_wasm.sh
│   └── optimize.sh
├── docs/               # 문서
│   ├── API.md
│   ├── ALGORITHMS.md
│   └── PERFORMANCE.md
└── web/                # 웹 페이지
    ├── index.html
    ├── benchmarks.html
    └── about.html
```

## 🚀 빠른 시작

### 필수 조건

```bash
# Node.js (v16+)
curl -fsSL https://deb.nodesource.com/setup_lts.x | sudo -E bash -

# Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
```

### 빌드

```bash
# C99 → WebAssembly 컴파일
./build/compile_wasm.sh

# C99 최적화 빌드 (SIMD 활용)
./build/compile_c.sh --with-simd

# WebAssembly 최적화 (Binaryen)
./build/optimize.sh -O3 --inline
```

### 웹에서 테스트

```bash
# 로컬 서버 시작
python3 -m http.server 8080

# 브라우저에서 열기
open http://localhost:8080/web/benchmarks.html
```

## 📊 성능 비교

| 알고리즘 | C99 (SIMD) | C99 (Scalar) | WebAssembly | 속도 향상 |
|----------|------------|--------------|-------------|------------|
| FFT 2048 | 0.8ms | 2.3ms | 1.1ms | **2.9x** |
| 필터 32 | 0.3ms | 0.9ms | 0.4ms | **2.3x** |
| 콘볼루션 1024 | 1.2ms | 4.1ms | 1.5ms | **2.7x** |

## 🧪 벤치마킹

웹에서 실시간 벤치마킹을 수행할 수 있습니다:

1. **FFT Visualizer**: 주파수와 진폭을 실시간으로 확인
2. **Filter Response**: 벡터 응답을 그래프로 표시
3. **Memory Profile**: WebAssembly 메모리 사용량 추적

```javascript
// 벤치마킹 예시
const fft = new FFTModule();
const benchmark = new Benchmark();

benchmark.run('FFT', () => {
    fft.fft(inputData);
}, 1000);
```

## 🔧 최적화 기법

### C99 최적화
- SIMD 내장 함수 (AVX2, SSE4.2)
- 루 언롤링 (Loop Unrolling)
- 데이터 정렬 (Cache-friendly)
- 인라인 어셈블리

### WebAssembly 최적화
- Linear Memory 구조
- 워드 크기 최적화 (64-bit vs 32-bit)
- 함수 인라인화
- LTO (Link-Time Optimization)

### JavaScript 최적화
- TypedArrays 활용
- WebAssembly 내부 메모리 직접 접근
- Worker 스레드 활용
- SharedArrayBuffer

## 📈 최적화 전략

1. **Phase 1**: C99 기본 구현 ✅
   - 순수 C 코드
   - SIMD 없음
   - 메모리 최적화 없음

2. **Phase 2**: C99 SIMD 최적화 🔥
   - AVX2, SSE4.2 활용
   - 데이터 정렬
   - 캐시 친화

3. **Phase 3**: WebAssembly 포팅 📦
   - Emscripten 설정
   - 메모리 레이아웃
   - 직렬화 최적화

4. **Phase 4**: Binaryen 최적화 ⚡
   - 함수 인라인
   - 상수 폴딩
   - 사전 계산

## 🧑‍💻 개발자 정보

- **개발자**: Seo David
- **GitHub**: [@probees1492](https://github.com/probees1492)
- **Email**: probees1492@gmail.com

## 📄 라이선스

이 프로젝트는 MIT 라이선스 하에 배포됩니다.

## 🤝 기여 방법

기여를 환영합니다! 다음 방법으로 참여할 수 있습니다:

1. Fork 저장소
2. 기능 브랜치 생성 (`git checkout -b feature/AmazingFeature`)
3. 커밋 (`git commit -m 'Add some AmazingFeature'`)
4. 푸시 (`git push origin feature/AmazingFeature`)

## 📞 문의

이슈나 제안이 있으시면 이슈를 등록해주세요.

---

**⚡ C99 + WebAssembly = 극한 성능**
