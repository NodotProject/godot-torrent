# Building Godot-Torrent from Source

Simple guide to building the Godot-Torrent GDExtension from source.

## Prerequisites

- **Git** - For cloning submodules
- **Python 3.6+** - For SCons
- **SCons 4.0+** - Build system
- **CMake 3.16+** - For libtorrent
- **C++ Compiler** with C++17 support (GCC 7+, Clang 6+, or MSVC 2019+)
- **Boost 1.67+** - For libtorrent
- **OpenSSL 1.1+** - For encryption

---

## Quick Start

### Linux

```bash
# Clone repository
git clone --recursive https://github.com/NodotProject/godot-torrent.git
cd godot-torrent

# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install build-essential scons cmake git \
    libboost-all-dev libssl-dev pkg-config

# Build
./build_local.sh linux

# Verify
ls -lh addons/godot-torrent/bin/libgodot-torrent.so
```

### Windows (Cross-Compile from Linux)

```bash
# Install MinGW cross-compiler
sudo apt-get install mingw-w64

# Build
./build_local.sh windows

# Verify
ls -lh addons/godot-torrent/bin/libgodot-torrent.dll
```

### macOS

```bash
# Install dependencies
brew install scons cmake boost openssl

# Build
./build_local.sh macos

# Verify
ls -lh addons/godot-torrent/bin/libgodot-torrent.dylib
```

---

## Build Instructions

### 1. Clone the Repository

```bash
git clone --recursive https://github.com/NodotProject/godot-torrent.git
cd godot-torrent
```

**Important**: Use `--recursive` to clone submodules. If you forgot:
```bash
git submodule update --init --recursive
```

### 2. Install Dependencies

**Ubuntu/Debian**:
```bash
sudo apt-get update
sudo apt-get install build-essential scons cmake git \
    pkg-config libboost-all-dev libssl-dev
```

**Fedora/RHEL**:
```bash
sudo dnf install gcc-c++ scons cmake git pkgconfig \
    boost-devel openssl-devel
```

**macOS**:
```bash
brew install scons cmake boost openssl
```

**Windows**:
Use WSL2 and follow Linux instructions, or install [MSYS2](https://www.msys2.org/) and:
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-boost mingw-w64-x86_64-openssl python-scons git
```

### 3. Build

```bash
# Linux
./build_local.sh linux

# Windows (cross-compile from Linux with MinGW)
sudo apt-get install mingw-w64
./build_local.sh windows

# macOS
./build_local.sh macos
```

### 4. Verify

```bash
ls -lh addons/godot-torrent/bin/
# Should see libgodot-torrent.{so,dll,dylib}
```

---

## Build Options

```bash
# Debug build (faster compilation, includes debug symbols)
scons platform=linux target=template_debug

# Release build (optimized)
scons platform=linux target=template_release

# Parallel build (use all CPU cores)
scons -j$(nproc)

# Clean build
scons --clean
```

---

## Troubleshooting

**"Boost not found"**
```bash
# Ubuntu/Debian
sudo apt-get install libboost-all-dev

# macOS
brew install boost
```

**"godot-cpp not found"**
```bash
git submodule update --init --recursive
```

**"Permission denied"**
```bash
chmod +x build_local.sh run_tests.sh
```

**Build fails**
```bash
# Clean and rebuild
rm -rf godot-cpp/bin libtorrent/build
./build_local.sh linux
```

---

## Next Steps

After building:
1. [Getting Started Guide](getting-started.md) - Learn to use the library
2. Run tests: `./run_tests.sh`
3. [API Reference](api-reference.md) - Explore the API

**Need help?** Open an [issue](https://github.com/NodotProject/godot-torrent/issues) with your build output.
