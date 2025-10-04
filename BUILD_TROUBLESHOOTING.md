# Build Troubleshooting Guide

This guide helps resolve common build issues with the Godot-Torrent extension.

## Prerequisites Issues

### Missing Build Tools

**Error**: `command not found: scons/cmake/g++`

**Solution**:
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential scons cmake pkg-config

# macOS
brew install scons cmake

# Windows (WSL or MSYS2)
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-scons
```

### Missing Boost Libraries

**Error**: `Could NOT find Boost (missing: Boost_INCLUDE_DIR)`

**Solution**:
```bash
# Ubuntu/Debian (Recommended for full functionality)
sudo apt-get install libboost-all-dev libssl-dev

# macOS
brew install boost openssl

# Alternative: Continue with stub library (limited functionality)
# The build system will automatically create a stub library if Boost is missing
```

**Note**: Without Boost, the build system creates a minimal stub library for development purposes. This satisfies linking requirements but provides limited torrent functionality.

## Build Failures

### LibTorrent Compilation Issues

**Symptoms**:
- CMake configuration fails
- Compilation errors related to Boost headers
- Missing symbol errors

**Solutions**:

1. **Install Boost dependencies** (recommended):
   ```bash
   sudo apt-get install libboost-all-dev libssl-dev
   ```

2. **Use stub library** (development mode):
   The build system automatically falls back to a stub library when dependencies are missing.

3. **Clean and rebuild**:
   ```bash
   rm -rf libtorrent/build godot-cpp/bin
   ./build_local.sh linux
   ```

### Linking Errors

**Error**: `undefined reference to libtorrent symbols`

**Diagnosis**:
```bash
# Check if libtorrent library exists and has reasonable size
ls -la libtorrent/build/libtorrent-rasterbar.a
# Should be > 1KB for stub, > 1MB for full library

# Check what symbols are in the library
nm -D libtorrent/build/libtorrent-rasterbar.a | head
```

**Solutions**:
1. Rebuild libtorrent: `rm libtorrent/build/libtorrent-rasterbar.a && ./build_local.sh linux`
2. Check system dependencies: `ldd addons/godot-torrent/bin/libgodot-torrent.so`

### Cross-Platform Issues

#### Windows Cross-Compilation

**Error**: `x86_64-w64-mingw32-gcc: command not found`

**Solution**:
```bash
# Ubuntu/Debian
sudo apt-get install mingw-w64

# Set environment variables if needed
export CC=x86_64-w64-mingw32-gcc
export CXX=x86_64-w64-mingw32-g++
```

#### macOS Universal Binary

**Error**: Architecture mismatch on Apple Silicon

**Solution**:
```bash
# Use universal build
./build_local.sh macos

# Or specify architecture
arch -x86_64 ./build_local.sh macos
```

## Cache Issues

### Godot-CPP Cache Problems

**Symptoms**:
- "godot-cpp cache miss" despite previous builds
- SCons rebuilds everything each time

**Solutions**:
```bash
# Check cache status
./build_local.sh linux  # Will show cache status

# Force godot-cpp rebuild
rm -rf godot-cpp/bin godot-cpp/.sconsign.dblite
./build_local.sh linux

# Check generated bindings
ls -la godot-cpp/gen/include godot-cpp/gen/src
```

### LibTorrent Cache Problems

**Symptoms**:
- "libtorrent cache miss" on every build
- Library file exists but cache checking fails

**Diagnosis**:
```bash
# Check library file
ls -la libtorrent/build/libtorrent-rasterbar.a

# Check if it's too small (stub vs real library)
du -h libtorrent/build/libtorrent-rasterbar.a
```

## Performance Issues

### Slow Builds

**Solutions**:
1. **Use caching**: Ensure both godot-cpp and libtorrent are cached
2. **Enable parallel builds**: Build system uses `make -j$(nproc)` automatically
3. **Use SSD storage**: Builds are I/O intensive
4. **Set SCons cache**: `export SCONS_CACHE_DIR=/path/to/cache`

### Memory Issues

**Error**: `g++: fatal error: Killed signal terminated program cc1plus`

**Solution**:
```bash
# Reduce parallel jobs for low-memory systems
export MAKEFLAGS="-j2"  # Use only 2 parallel jobs
./build_local.sh linux
```

## Validation

### Verify Successful Build

```bash
# Check library exists and has reasonable size
ls -lh addons/godot-torrent/bin/libgodot-torrent.so
# Should be ~1-2MB for stub, larger for full implementation

# Check dependencies
ldd addons/godot-torrent/bin/libgodot-torrent.so

# Verify symbols
nm -D addons/godot-torrent/bin/libgodot-torrent.so | grep torrent
```

### Test in Godot

```bash
# Run basic test
godot --headless -s test_basic.gd

# Check for loading errors
godot --headless --print-fps -s test_basic.gd 2>&1 | grep -i error
```

## Common Error Messages

### "SCons error"
- Check SCons installation: `scons --version`
- Verify SConstruct file is present
- Check file permissions

### "undefined symbol"
- Usually indicates incomplete linking
- Rebuild both godot-cpp and libtorrent
- Check that all required system libraries are linked

### "Permission denied"
- Check file permissions: `ls -la`
- Ensure build directories are writable
- On WSL, check Windows filesystem mounting options

## Getting Help

If issues persist:

1. **Check build logs**: Save full output of `./build_local.sh linux 2>&1 | tee build.log`
2. **System information**: `uname -a`, compiler versions, dependency versions
3. **File an issue**: Include build logs, system info, and error messages
4. **Community help**: GitHub Discussions or Godot community forums

## Build Configuration

### Environment Variables

```bash
# Compiler selection
export CC=gcc-11
export CXX=g++-11

# SCons cache directory
export SCONS_CACHE_DIR=/tmp/scons_cache

# Cross-compilation
export CC=x86_64-w64-mingw32-gcc
export CXX=x86_64-w64-mingw32-g++
```

### Build Variants

```bash
# Development build with debug info
scons platform=linux target=template_debug

# Release build (default)
scons platform=linux target=template_release

# Custom architecture
scons platform=linux arch=arm64
```