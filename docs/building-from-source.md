# Building Godot-Torrent from Source

Complete guide to building the Godot-Torrent GDExtension from source code.

## Prerequisites

### Required Tools

- **Git** - For cloning and submodule management
- **Python 3.6+** - For SCons build system
- **SCons 4.0+** - Build system
- **CMake 3.16+** - For building libtorrent
- **C++ Compiler** with C++17 support:
  - Linux: GCC 7+ or Clang 6+
  - Windows: MinGW-w64 or MSVC 2019+
  - macOS: Xcode Command Line Tools

### Optional Dependencies (Recommended)

- **Boost 1.67+** - For full libtorrent functionality
- **OpenSSL 1.1+** - For encryption support

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

## Detailed Build Instructions

### Step 1: Clone the Repository

```bash
git clone --recursive https://github.com/NodotProject/godot-torrent.git
cd godot-torrent
```

**Important**: Use `--recursive` to clone submodules (godot-cpp and libtorrent).

If you forgot `--recursive`:
```bash
git submodule update --init --recursive
```

### Step 2: Install Dependencies

#### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install \
    build-essential \
    scons \
    cmake \
    git \
    pkg-config \
    libboost-all-dev \
    libssl-dev
```

#### Fedora/RHEL

```bash
sudo dnf install \
    gcc-c++ \
    scons \
    cmake \
    git \
    pkgconfig \
    boost-devel \
    openssl-devel
```

#### macOS

```bash
# Install Homebrew if needed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install scons cmake boost openssl
```

#### Windows

**Option 1: Use WSL2** (Recommended)
```bash
# Install WSL2
wsl --install

# Follow Linux instructions inside WSL
```

**Option 2: Native Windows**
- Install [MSYS2](https://www.msys2.org/)
- Install dependencies via pacman:
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-boost mingw-w64-x86_64-openssl \
    python-scons git
```

### Step 3: Build

#### Automated Build (Recommended)

```bash
# Build for your platform
./build_local.sh linux    # Linux
./build_local.sh windows  # Windows
./build_local.sh macos    # macOS

# With clean build
./build_local.sh linux clean
```

#### Manual Build

```bash
# 1. Build godot-cpp
cd godot-cpp
scons platform=linux target=template_debug
cd ..

# 2. Build libtorrent
mkdir -p libtorrent/build
cd libtorrent/build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DBUILD_SHARED_LIBS=ON \
         -Ddeprecated-functions=OFF
cmake --build . --parallel
cd ../..

# 3. Build godot-torrent
scons platform=linux target=template_debug
```

### Step 4: Verify Build

```bash
# Check if library was created
ls -lh addons/godot-torrent/bin/

# Should see:
# libgodot-torrent.linux.template_debug.x86_64.so    # Linux
# libgodot-torrent.windows.template_debug.x86_64.dll # Windows
# libgodot-torrent.macos.template_debug.universal.dylib # macOS
```

### Step 5: Test

```bash
# Run tests
./run_tests.sh

# Or manually
godot --headless -s addons/gut/gut_cmdln.gd -gdir=test/unit -gexit
```

---

## Build Options

### SCons Options

```bash
# Platform
scons platform=linux      # linux, windows, macos
scons platform=windows target=template_release

# Target
scons target=template_debug    # Debug build
scons target=template_release  # Release build

# Architecture
scons arch=x86_64    # 64-bit (default)
scons arch=x86_32    # 32-bit

# Parallel build
scons -j8            # Use 8 CPU cores

# Verbose output
scons verbose=yes

# Clean build
scons --clean
```

### CMake Options (libtorrent)

```bash
cd libtorrent/build

# Build type
cmake .. -DCMAKE_BUILD_TYPE=Release  # or Debug

# Shared library
cmake .. -DBUILD_SHARED_LIBS=ON

# Disable deprecated functions
cmake .. -Ddeprecated-functions=OFF

# Enable/disable encryption
cmake .. -Dencryption=ON

# Custom install prefix
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
```

---

## Build Configurations

### Development Build (Fast iteration)

```bash
# Use cached dependencies
./build_simple.sh

# Or manually
scons platform=linux target=template_debug -j$(nproc)
```

### Production Build (Optimized)

```bash
# Full optimized build
scons platform=linux target=template_release optimize=speed -j$(nproc)
```

### Debug Build (With symbols)

```bash
# Debug symbols included
scons platform=linux target=template_debug debug_symbols=yes
```

---

## Troubleshooting

### "Boost not found"

**Solution**: Install Boost or use fallback build:
```bash
# Ubuntu/Debian
sudo apt-get install libboost-all-dev

# macOS
brew install boost

# Fallback (no Boost) - limited functionality
./build_local.sh linux # Will use stub fallback
```

### "godot-cpp not found"

**Solution**: Initialize submodules:
```bash
git submodule update --init --recursive
```

### "Permission denied"

**Solution**: Make scripts executable:
```bash
chmod +x build_local.sh run_tests.sh
```

### Build fails with "undefined reference"

**Causes**:
- Missing dependencies
- Wrong library paths
- Incompatible boost version

**Solutions**:
```bash
# Check dependencies
pkg-config --libs --cflags openssl
ldconfig -p | grep boost

# Clean and rebuild
rm -rf godot-cpp/bin libtorrent/build
./build_local.sh linux clean
```

### Slow build times

**Solutions**:
```bash
# Use parallel builds
scons -j$(nproc)  # Use all CPU cores

# Use ccache (if available)
export CXX="ccache g++"
scons platform=linux

# Cache dependencies
# godot-cpp and libtorrent are automatically cached
```

---

## Build Performance

### Typical Build Times

| Configuration | First Build | Cached Build |
|--------------|-------------|--------------|
| Debug | 3-5 minutes | < 30 seconds |
| Release | 4-6 minutes | < 45 seconds |
| Clean Debug | 2-4 minutes | 2-4 minutes |

*On a modern 8-core CPU*

### Optimizing Build Speed

1. **Use parallel builds**:
```bash
scons -j$(nproc)  # Linux/macOS
scons -j%NUMBER_OF_PROCESSORS%  # Windows
```

2. **Use ccache**:
```bash
sudo apt-get install ccache
export CXX="ccache g++"
```

3. **Don't clean unless necessary** - Incremental builds are much faster

4. **Use SSD** - Significantly faster than HDD

---

## Cross-Compilation

### Windows from Linux

```bash
# Install MinGW
sudo apt-get install mingw-w64

# Build
scons platform=windows arch=x86_64 target=template_release
```

### macOS from Linux (OSXCross)

```bash
# Setup OSXCross (one-time)
git clone https://github.com/tpoechtrager/osxcross
# Follow OSXCross setup instructions

# Build
export OSXCROSS_ROOT=/path/to/osxcross
scons platform=macos arch=universal
```

---

## CI/CD Integration

### GitHub Actions Example

```yaml
name: Build

on: [push, pull_request]

jobs:
  build-linux:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
        with:
          submodules: recursive

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y libboost-all-dev libssl-dev

      - name: Build
        run: ./build_local.sh linux

      - name: Test
        run: ./run_tests.sh
```

---

## Build Artifacts

### Output Files

```
addons/godot-torrent/bin/
├── libgodot-torrent.linux.template_debug.x86_64.so
├── libgodot-torrent.linux.template_release.x86_64.so
├── libgodot-torrent.windows.template_debug.x86_64.dll
├── libgodot-torrent.windows.template_release.x86_64.dll
├── libgodot-torrent.macos.template_debug.universal.dylib
└── libgodot-torrent.macos.template_release.universal.dylib
```

### Library Sizes

| Platform | Debug | Release |
|----------|-------|---------|
| Linux | ~3 MB | ~1.4 MB |
| Windows | ~3.5 MB | ~1.6 MB |
| macOS | ~4 MB | ~2 MB |

---

## Advanced Topics

### Custom libtorrent Build

```bash
cd libtorrent
mkdir build-custom
cd build-custom

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -Dencryption=ON \
    -Ddeprecated-functions=OFF \
    -DCMAKE_CXX_FLAGS="-march=native -O3"

cmake --build . --parallel
```

### Debugging Build Issues

```bash
# Verbose build
scons platform=linux verbose=yes

# Check symbols
nm addons/godot-torrent/bin/libgodot-torrent.so | grep TorrentSession

# Check dependencies
ldd addons/godot-torrent/bin/libgodot-torrent.so  # Linux
otool -L addons/godot-torrent/bin/libgodot-torrent.dylib  # macOS
```

---

## Next Steps

After building:
1. [Getting Started Guide](getting-started.md) - Learn to use the library
2. [Run Tests](run_tests.sh) - Verify your build
3. [API Reference](api-reference.md) - Explore the API

---

**Need help?** Open an [issue](https://github.com/NodotProject/godot-torrent/issues) with your build output.
