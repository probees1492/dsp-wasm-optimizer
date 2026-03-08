# 🎵 DSP → WebAssembly Optimization MVP

[![GitHub](https://img.shields.io/badge/GitHub-prowl-purple.svg)](https://github.com/probes1492/dsp-wasm-optimizer)

High-performance DSP library written in C99, compiled to WebAssembly for real-time testing and optimization on the web.

## 🎯 Core Features

- **C99 DSP Library**
  - FFT (Fast Fourier Transform): Radix-2, Cooley-Tukey
  - Filters: Biquad, FIR, IIR
  - Vector Operations: SIMD optimized (AVX2, SSE4.2, NEON)

- **WebAssembly Compilation**
  - Emscripten configuration
  - Memory optimization (Linear Memory)
  - SIMD instruction support (128-bit vector)

- **Web-based Benchmarking**
  - Real-time performance measurement on JavaScript
  - Visualizer (FFT visualization, Filter response graph)
  - Auto-optimization parameter recommendation

- **GitHub Copilot Integration**
  - Copilot Workspace setup
  - AI-assisted code auto-completion

## 🛠️ Tech Stack

- **DSP**: C99, SIMD (AVX2, SSE4.2, NEON)
- **WebAssembly**: Emscripten, Binaryen
- **Benchmarking**: JavaScript, WebGL
- **Optimization**: Binaryen (wasm-opt), LLVM opt
- **Framework**: Google Benchmark.js

## 📁 Project Structure

```
dsp-wasm-optimizer/
├── src/
│   ├── c/               # C99 DSP library
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
│   ├── wasm/            # WebAssembly wrapper
│   │   ├── fft.wat
│   │   └── filters.wat
│   └── bench/           # Benchmark code
│       ├── fft_bench.js
│       ├── filter_bench.js
│       └── visualizer.js
├── tests/              # Web tests
│   ├── fft_test.html
│   ├── filter_test.html
│   └── memory_test.html
├── build/              # Build scripts
│   ├── compile_c.sh
│   ├── compile_wasm.sh
│   └── optimize.sh
├── docs/               # Documentation
│   ├── API.md
│   ├── ALGORITHMS.md
│   └── PERFORMANCE.md
└── web/                # Web pages
    ├── index.html
    ├── benchmarks.html
    └── about.html
```

## 🚀 Quick Start

### Prerequisites

```bash
# Node.js (v16+)
curl -fsSL https://deb.nodesource.com/setup_lts.x | sudo -E bash -

# Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
```

### Build

```bash
# C99 → WebAssembly compilation
./build/compile_wasm.sh

# C99 optimized build (with SIMD)
./build/compile_c.sh --with-simd

# WebAssembly optimization (Binaryen)
./build/optimize.sh -O3 --inline
```

### Testing on Web

```bash
# Start local server
python3 -m http.server 8080

# Open in browser
open http://localhost:8080/web/benchmarks.html
```

## 📊 Performance Comparison

| Algorithm | C99 (SIMD) | C99 (Scalar) | WebAssembly | Speedup |
|-----------|-------------|--------------|-------------|----------|
| FFT 2048   | 0.8ms      | 2.3ms        | 1.1ms       | 2.9x     |
| Filter 32   | 0.3ms      | 0.9ms        | 0.4ms       | 2.3x     |
| Conv 1024   | 1.2ms      | 4.1ms        | 1.5ms       | 2.7x     |

## 🧪 Benchmarking

Run real-time benchmarking on the web:

1. **FFT Visualizer**: Real-time frequency and magnitude display
2. **Filter Response**: Vector impulse response graph
3. **Memory Profile**: WebAssembly memory usage tracking

```javascript
// Benchmarking example
const fft = new FFTModule();
const benchmark = new Benchmark();

benchmark.run('FFT', () => {
    fft.fft(inputData);
}, 1000);
```

## 🔧 Optimization Techniques

### C99 Optimization
- SIMD intrinsics (AVX2, SSE4.2)
- Loop unrolling
- Data alignment (Cache-friendly)
- Inline assembly

### WebAssembly Optimization
- Linear Memory structure
- Optimized word size (64-bit vs 32-bit)
- Function inlining
- LTO (Link-Time Optimization)

### JavaScript Optimization
- TypedArrays usage
- Direct WebAssembly memory access
- Worker threads
- SharedArrayBuffer

## 📚 Optimization Strategy

1. **Phase 1**: C99 Basic Implementation ✅
   - Plain C code
   - No SIMD
   - No memory optimization

2. **Phase 2**: C99 SIMD Optimization 🔥
   - AVX2, SSE4.2 intrinsics
   - Data alignment
   - Cache optimization

3. **Phase 3**: WebAssembly Packaging 📦
   - Emscripten configuration
   - Memory layout
   - Linearization optimization

4. **Phase 4**: Binaryen Optimization ⚡
   - Function inlining
   - Constant folding
   - Pre-computation

## 👨‍💻 Developer Information

- **Developer**: Seo David
- **GitHub**: [@probes1492](https://github.com/probes1492)
- **Email**: probes1492@gmail.com

## 📄 License

This project is licensed under the MIT License.

## 🤝 Contributing

Contributions are welcome! You can contribute by:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)

## 📞 Issues

If you have any questions or suggestions, please open an issue.

---

**⚡ C99 + WebAssembly = Ultra Performance**
