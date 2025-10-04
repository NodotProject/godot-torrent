#!/bin/bash

# Enhanced build script for Godot-Torrent (libtorrent GDExtension)
# This script mimics the caching behavior from .github/workflows/build_release.yml

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# --- Configuration ---
PLATFORM=""
ARCH="x86_64"
SCONS_FLAGS=""

# --- Helper Functions ---
show_usage() {
    echo -e "${YELLOW}Usage: $0 [linux|macos|windows]${NC}"
    echo "  linux: Build for Linux (x86_64)"
    echo "  macos: Build for macOS (universal)"
    echo "  windows: Build for Windows (x86_64, cross-compile)"
    exit 1
}

# --- Platform-specific Setup ---
setup_linux() {
    PLATFORM="linux"
    ARCH="x86_64"
    SCONS_FLAGS="platform=linux"
    echo -e "${BLUE}=== Godot-Torrent Local Build Script (Linux) ===${NC}"
}

setup_macos() {
    PLATFORM="macos"
    ARCH="universal"
    SCONS_FLAGS="platform=macos arch=universal"
    echo -e "${BLUE}=== Godot-Torrent Local Build Script (macOS) ===${NC}"
}

setup_windows() {
    PLATFORM="windows"
    ARCH="x86_64"
    SCONS_FLAGS="platform=windows use_mingw=yes"
    echo -e "${BLUE}=== Godot-Torrent Local Build Script (Windows Cross-Compile) ===${NC}"
}

# --- Build Functions ---

# Function to check if godot-cpp cache is valid
check_godotcpp_cache() {
    echo -e "${YELLOW}Checking godot-cpp cache...${NC}"
    
    # When cross-compiling with MinGW, we still get .a files, not .lib files
    # .lib files are only produced when building with MSVC on Windows
    local lib_ext="a"

    local required_files=(
        "godot-cpp/bin/libgodot-cpp.${PLATFORM}.template_release.${ARCH}.${lib_ext}"
        "godot-cpp/bin/libgodot-cpp.${PLATFORM}.template_debug.${ARCH}.${lib_ext}"
        "godot-cpp/gen/include"
        "godot-cpp/gen/src"
    )
    
    for file in "${required_files[@]}"; do
        if [ ! -e "$file" ]; then
            echo -e "${RED}Cache miss: $file not found${NC}"
            return 1
        fi
    done
    
    if [ -f "godot-cpp/.sconsign.dblite" ]; then
        echo -e "${GREEN}SCons signature file found${NC}"
    fi
    
    echo -e "${GREEN}godot-cpp cache is valid!${NC}"
    return 0
}

# Function to build libtorrent
build_libtorrent() {
    echo -e "${YELLOW}Building libtorrent for ${PLATFORM}...${NC}"

    cd libtorrent

    # Check if we need to switch to compatible branch
    local current_branch=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown")
    echo -e "${BLUE}Current libtorrent branch: ${current_branch}${NC}"

    # Check for Boost version compatibility
    if [ -f /usr/include/boost/version.hpp ]; then
        local boost_version=$(grep "BOOST_LIB_VERSION" /usr/include/boost/version.hpp | cut -d'"' -f2 | tr -d '_')
        echo -e "${BLUE}Detected Boost version: ${boost_version}${NC}"

        # Boost 1.83+ has compatibility issues with libtorrent RC_2_0
        # Use RC_1_2 for better stability
        if [ "$current_branch" != "RC_1_2" ]; then
            echo -e "${YELLOW}Switching to RC_1_2 branch for better Boost compatibility...${NC}"
            git stash > /dev/null 2>&1 || true
            git checkout RC_1_2 2>&1 | grep -v "warning: unable to rmdir" || true
        fi
    fi

    # Create/clean build directory
    echo -e "${YELLOW}Setting up build directory...${NC}"
    rm -rf build
    mkdir -p build
    cd build

    # Check for Boost dependencies first
    echo -e "${YELLOW}Checking for Boost dependencies...${NC}"
    if ! pkg-config --exists boost 2>/dev/null \
        && [ ! -d /usr/include/boost ] \
        && [ ! -d /usr/local/include/boost ] \
        && ! find /usr/include -name "boost" -type d 2>/dev/null | grep -q boost; then
        echo -e "${RED}Boost development libraries not found!${NC}"
        echo -e "${YELLOW}Please install Boost development libraries:${NC}"
        if [[ "$OSTYPE" == "linux-gnu"* ]]; then
            echo "  sudo apt-get install libboost-all-dev libssl-dev"
        elif [[ "$OSTYPE" == "darwin"* ]]; then
            echo "  brew install boost openssl"
        fi
        echo ""
        echo -e "${YELLOW}Creating stub library...${NC}"
        create_libtorrent_stub
        cd ../..
        return
    fi

    # Configure libtorrent build based on platform
    # IMPORTANT: Always use -DCMAKE_POSITION_INDEPENDENT_CODE=ON for shared library linking
    local cmake_flags=""
    if [ "$PLATFORM" == "linux" ]; then
        cmake_flags="-DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON"
    elif [ "$PLATFORM" == "macos" ]; then
        cmake_flags="-DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES='x86_64;arm64' -DCMAKE_POSITION_INDEPENDENT_CODE=ON"
    elif [ "$PLATFORM" == "windows" ]; then
        cmake_flags="-DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64-x86_64.cmake -DCMAKE_POSITION_INDEPENDENT_CODE=ON"
        export CC=x86_64-w64-mingw32-gcc
        export CXX=x86_64-w64-mingw32-g++
    fi

    # Try to build with reduced dependencies
    echo -e "${YELLOW}Configuring CMake...${NC}"
    if cmake .. $cmake_flags \
        -DBUILD_SHARED_LIBS=OFF \
        -Dbuild_tests=OFF \
        -Dbuild_examples=OFF \
        -Dbuild_tools=OFF; then

        echo -e "${GREEN}CMake configuration successful${NC}"
        echo -e "${YELLOW}Building libtorrent (this may take several minutes)...${NC}"

        # Use parallel build with progress indication
        local nproc_count=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo "4")
        if make -j${nproc_count} 2>&1 | tail -20; then
            # Verify the library was actually built and is not tiny
            if [ -f "libtorrent-rasterbar.a" ]; then
                local lib_size=$(wc -c < "libtorrent-rasterbar.a" 2>/dev/null | tr -d '[:space:]' || echo "0")
                if [ "$lib_size" -gt 1048576 ]; then  # > 1MB
                    echo -e "${GREEN}libtorrent build completed successfully! (${lib_size} bytes)${NC}"
                else
                    echo -e "${YELLOW}Warning: Library seems too small (${lib_size} bytes), may be incomplete${NC}"
                fi
            else
                echo -e "${RED}Library file not found after build${NC}"
            fi
        else
            echo -e "${RED}libtorrent build failed!${NC}"
            echo -e "${YELLOW}Trying to create a minimal stub library...${NC}"
            create_libtorrent_stub
        fi
    else
        echo -e "${RED}CMake configuration failed!${NC}"
        echo -e "${YELLOW}Creating a minimal stub library for development...${NC}"
        create_libtorrent_stub
    fi

    cd ../..
}

# Function to create a minimal libtorrent stub for development
create_libtorrent_stub() {
    echo -e "${YELLOW}Creating minimal libtorrent stub...${NC}"
    
    # Create a minimal source file with proper error handling
    cat > libtorrent_stub.cpp << 'EOF'
// Minimal libtorrent stub for development
// This provides basic symbols to satisfy linking requirements
#include <functional>
#include <string>
#include <stdexcept>
#include <memory>

namespace libtorrent {
    // Forward declarations
    struct session_params {};
    struct add_torrent_params {};
    
    class torrent_handle {
    public:
        torrent_handle() = default;
        ~torrent_handle() = default;
        bool is_valid() const { return false; }
    };
    
    class torrent_info {
    public:
        torrent_info() = default;
        ~torrent_info() = default;
        bool is_valid() const { return false; }
    };
    
    class torrent_status {
    public:
        torrent_status() = default;
        ~torrent_status() = default;
        float progress() const { return 0.0f; }
        bool is_paused() const { return false; }
    };
    
    class peer_info {
    public:
        peer_info() = default;
        ~peer_info() = default;
    };
    
    class session {
    private:
        bool m_valid = false;
        
    public:
        session() : m_valid(true) {}
        ~session() { m_valid = false; }
        
        // Safer alert handling
        void set_alert_notify(std::function<void()> const& fn) {
            // Store function but don't call it in stub mode
            (void)fn; // Suppress unused parameter warning
        }
        
        bool is_valid() const { return m_valid; }
        
        // Basic session management
        void start() { m_valid = true; }
        void stop() { m_valid = false; }
    };
    
    class alert {
    public:
        virtual ~alert() = default;
        virtual int type() const { return 0; }
        virtual char const* what() const { return "stub_alert"; }
        virtual std::string message() const { return "stub_alert"; }
    };
    
    // Provide some basic functions that might be called
    inline std::string version() { return "stub-2.0.0"; }
    
    // Export some symbols that might be needed for linking
    extern "C" {
        // Basic version info
        const char* libtorrent_version_string() {
            return "2.0.0-stub";
        }
        
        int libtorrent_version_major() { return 2; }
        int libtorrent_version_minor() { return 0; }
        int libtorrent_version_patch() { return 0; }
        
        // Dummy initialization functions
        void libtorrent_init() {}
        void libtorrent_cleanup() {}
    }
}

// Additional safety: define some common symbols that might be missing
extern "C" {
    // Boost-related stubs (if needed)
    void boost_system_error_category() {}
    void boost_system_generic_category() {}
}
EOF
    
    # Compile stub library with better error handling
    if ${CXX:-g++} -fPIC -c libtorrent_stub.cpp -std=c++17 -O2 -o libtorrent_stub.o; then
        if ar rcs libtorrent-rasterbar.a libtorrent_stub.o; then
            rm -f libtorrent_stub.cpp libtorrent_stub.o
            echo -e "${GREEN}Stub library created: libtorrent-rasterbar.a${NC}"
            echo -e "${YELLOW}Note: This is a development stub. Real libtorrent functionality requires proper Boost dependencies.${NC}"
        else
            echo -e "${RED}Failed to create archive${NC}"
            return 1
        fi
    else
        echo -e "${RED}Failed to compile stub${NC}"
        return 1
    fi
}

# Function to check if libtorrent cache is valid
check_libtorrent_cache() {
    echo -e "${YELLOW}Checking libtorrent cache...${NC}"

    local lib_ext="a"

    local required_files=(
        "libtorrent/build/libtorrent-rasterbar.${lib_ext}"
        "libtorrent/include"
    )

    for file in "${required_files[@]}"; do
        if [ ! -e "$file" ]; then
            echo -e "${RED}Cache miss: $file not found${NC}"
            return 1
        fi
    done

    # Check library file size (should be > 1MB for real library, > 1KB for stub)
    local lib_file="libtorrent/build/libtorrent-rasterbar.${lib_ext}"
    if [ -f "$lib_file" ]; then
        local file_size=$(wc -c < "$lib_file" 2>/dev/null | tr -d '[:space:]' || echo "0")

        # Check if it's a real library (> 1MB) or just a stub (< 100KB)
        if [ "$file_size" -gt 1048576 ]; then
            echo -e "${GREEN}libtorrent cache is valid! (${file_size} bytes - real library)${NC}"
            return 0
        elif [ "$file_size" -gt 1024 ]; then
            echo -e "${YELLOW}libtorrent stub library found (${file_size} bytes)${NC}"
            echo -e "${YELLOW}To build full libtorrent, delete libtorrent/build and re-run${NC}"
            return 0
        else
            echo -e "${RED}libtorrent library file too small (${file_size} bytes), rebuilding...${NC}"
            return 1
        fi
    fi

    echo -e "${GREEN}libtorrent cache is valid!${NC}"
    return 0
}

# Function to build godot-cpp
build_godotcpp() {
    echo -e "${YELLOW}Building godot-cpp (cache miss)...${NC}"
    
    cd godot-cpp
    
    echo -e "${BLUE}Building template_release...${NC}"
    scons $SCONS_FLAGS generate_bindings=yes target=template_release
    
    echo -e "${BLUE}Building template_debug...${NC}"
    scons $SCONS_FLAGS generate_bindings=yes target=template_debug
    
    cd ..
    
    echo -e "${GREEN}godot-cpp build completed!${NC}"
}

# Function to install dependencies
install_dependencies() {
    echo -e "${YELLOW}Checking dependencies for ${PLATFORM}...${NC}"
    
    if [ "$PLATFORM" == "linux" ]; then
        if [[ "$OSTYPE" != "linux-gnu"* ]]; then
            echo -e "${RED}Linux build requires a Linux environment. Current OS: $OSTYPE${NC}"
            exit 1
        fi
        
        local required_tools=("scons" "g++" "pkg-config" "cmake" "make")
        local missing_tools=()
        
        for tool in "${required_tools[@]}"; do
            if ! command -v "$tool" &> /dev/null; then
                missing_tools+=("$tool")
            fi
        done
        
        if [ ${#missing_tools[@]} -ne 0 ]; then
            echo -e "${RED}Missing required tools: ${missing_tools[*]}${NC}"
            echo -e "${YELLOW}Please install them with:${NC}"
            echo "sudo apt-get update && sudo apt-get install -y build-essential scons pkg-config cmake libssl-dev"
            exit 1
        fi
        
        # Check for Boost (optional but recommended)
        if ! pkg-config --exists boost 2>/dev/null && ! find /usr/include -name "boost" -type d 2>/dev/null | grep -q boost; then
            echo -e "${YELLOW}WARNING: Boost development libraries not found${NC}"
            echo -e "${YELLOW}For full libtorrent functionality, install with:${NC}"
            echo "sudo apt-get install libboost-all-dev"
            echo -e "${YELLOW}Continuing with fallback stub library...${NC}"
            echo ""
        else
            echo -e "${GREEN}Boost development libraries found${NC}"
        fi
        
    elif [ "$PLATFORM" == "windows" ]; then
        # Check if we have the MinGW cross-compiler tools
        local required_tools=("scons" "cmake" "make")
        local missing_tools=()
        
        for tool in "${required_tools[@]}"; do
            if ! command -v "$tool" &> /dev/null; then
                missing_tools+=("$tool")
            fi
        done
        
        # Check for MinGW cross-compiler (use environment variables if set)
        local gcc_cmd="${CC:-x86_64-w64-mingw32-gcc}"
        local gxx_cmd="${CXX:-x86_64-w64-mingw32-g++}"
        
        if ! command -v "$gcc_cmd" &> /dev/null; then
            missing_tools+=("$gcc_cmd")
        fi
        
        if ! command -v "$gxx_cmd" &> /dev/null; then
            missing_tools+=("$gxx_cmd")
        fi
        
        if [ ${#missing_tools[@]} -ne 0 ]; then
            echo -e "${RED}Missing required tools for Windows cross-compilation: ${missing_tools[*]}${NC}"
            echo -e "${YELLOW}Please install MinGW-w64 with:${NC}"
            if [[ "$OSTYPE" == "linux-gnu"* ]]; then
                echo "sudo apt-get update && sudo apt-get install -y mingw-w64 scons cmake"
            elif [[ "$OSTYPE" == "darwin"* ]]; then
                echo "brew install mingw-w64 scons cmake"
            else
                echo "Please install MinGW-w64 cross-compiler for your system"
            fi
            exit 1
        fi
    elif [ "$PLATFORM" == "macos" ]; then
        if [[ "$OSTYPE" != "darwin"* ]]; then
            echo -e "${RED}macOS build requires a macOS environment. Current OS: $OSTYPE${NC}"
            exit 1
        fi
        
        local required_tools=("scons" "cmake" "make")
        local missing_tools=()
        
        for tool in "${required_tools[@]}"; do
            if ! command -v "$tool" &> /dev/null; then
                missing_tools+=("$tool")
            fi
        done
        
        if [ ${#missing_tools[@]} -ne 0 ]; then
            echo -e "${RED}Missing required tools: ${missing_tools[*]}${NC}"
            echo -e "${YELLOW}Please install them with:${NC}"
            echo "brew install scons cmake boost openssl"
            exit 1
        fi
    fi
    
    echo -e "${GREEN}All required dependencies are available${NC}"
}

# Function to build the main project
build_main_project() {
    echo -e "${YELLOW}Building main project...${NC}"
    scons $SCONS_FLAGS target=template_release
    echo -e "${GREEN}Main project build completed!${NC}"
}

# Function to package addon
package_addon() {
    echo -e "${YELLOW}Packaging addon...${NC}"
    
    local artifact_name="godot-torrent-${PLATFORM}-${ARCH}"
    
    mkdir -p "package/addons"
    cp -r "addons/godot-torrent" "package/addons/"
    
    # Correctly copy the library based on the platform
    if [ "$PLATFORM" == "macos" ]; then
        cp -r "bin/libgodot-torrent.macos.template_release.framework" "package/addons/godot-torrent/bin/" 2>/dev/null || \
        cp "addons/godot-torrent/bin/libgodot-torrent.dylib" "package/addons/godot-torrent/bin/" 2>/dev/null || \
        echo -e "${YELLOW}Warning: macOS library not found${NC}"
    elif [ "$PLATFORM" == "windows" ]; then
        cp "addons/godot-torrent/bin/libgodot-torrent.dll" "package/addons/godot-torrent/bin/" 2>/dev/null || \
        echo -e "${YELLOW}Warning: Windows library not found${NC}"
    elif [ "$PLATFORM" == "linux" ]; then
        cp "addons/godot-torrent/bin/libgodot-torrent.so" "package/addons/godot-torrent/bin/" 2>/dev/null || \
        echo -e "${YELLOW}Warning: Linux library not found${NC}"
    else
        echo -e "${YELLOW}Packaging for ${PLATFORM} is not fully implemented yet.${NC}"
    fi

    cd "package"
    zip -r "../${artifact_name}.zip" .
    cd ..
    
    rm -rf "package"
    
    echo -e "${GREEN}Addon packaged as ${artifact_name}.zip${NC}"
}

# Function to show cache status
show_cache_status() {
    echo -e "${BLUE}=== Cache Status ===${NC}"
    
    # Check libtorrent cache
    if [ -f "libtorrent/build/libtorrent-rasterbar.a" ] || [ -f "libtorrent/build/libtorrent-rasterbar.lib" ]; then
        echo -e "${GREEN}✓ libtorrent library cached${NC}"
    else
        echo -e "${RED}✗ libtorrent library missing${NC}"
    fi
    
    # When cross-compiling with MinGW, we still get .a files, not .lib files
    # .lib files are only produced when building with MSVC on Windows
    local lib_ext="a"

    if [ -f "godot-cpp/bin/libgodot-cpp.${PLATFORM}.template_release.${ARCH}.${lib_ext}" ]; then
        echo -e "${GREEN}✓ Release library cached${NC}"
    else
        echo -e "${RED}✗ Release library missing${NC}"
    fi
    
    if [ -f "godot-cpp/bin/libgodot-cpp.${PLATFORM}.template_debug.${ARCH}.${lib_ext}" ]; then
        echo -e "${GREEN}✓ Debug library cached${NC}"
    else
        echo -e "${RED}✗ Debug library missing${NC}"
    fi
    
    if [ -d "godot-cpp/gen/include" ] && [ -d "godot-cpp/gen/src" ]; then
        echo -e "${GREEN}✓ Generated bindings cached${NC}"
    else
        echo -e "${RED}✗ Generated bindings missing${NC}"
    fi
    
    if [ -f "godot-cpp/.sconsign.dblite" ]; then
        echo -e "${GREEN}✓ SCons signature file present${NC}"
    else
        echo -e "${YELLOW}! SCons signature file missing (will be created)${NC}"
    fi
    
    echo ""
}

# --- Main Execution ---
main() {
    # Parse command-line arguments
    if [ -z "$1" ]; then
        show_usage
    fi

    case "$1" in
        linux)
            setup_linux
            ;;
        macos)
            setup_macos
            ;;
        windows)
            export CC=x86_64-w64-mingw32-gcc
            export CXX=x86_64-w64-mingw32-g++
            setup_windows
            ;;
        *)
            show_usage
            ;;
    esac

    echo -e "${BLUE}Platform: ${PLATFORM}, Architecture: ${ARCH}${NC}"
    echo ""

    install_dependencies
    show_cache_status
    
    if ! check_libtorrent_cache; then
        build_libtorrent
    else
        echo -e "${GREEN}Using cached libtorrent build${NC}"
    fi
    
    if ! check_godotcpp_cache; then
        build_godotcpp
    else
        echo -e "${GREEN}Using cached godot-cpp build${NC}"
    fi
    
    echo ""
    build_main_project
    echo ""
    
    read -p "Do you want to package the addon? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        package_addon
    fi
    
    echo ""
    echo -e "${GREEN}=== Build Complete! ===${NC}"
    echo -e "${BLUE}Built for: ${PLATFORM} (${ARCH})${NC}"
    
    show_cache_status
}

main "$@"