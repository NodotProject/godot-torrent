from SCons.Script import ARGUMENTS, Environment, Mkdir, Default, File, CacheDir
import os
import sys

# Resolve platform/target/arch from args (matches CI defaults)
platform = ARGUMENTS.get('platform')
if not platform:
    p = sys.platform
    if p.startswith('win'):
        platform = 'windows'
    elif p == 'darwin':
        platform = 'macos'
    else:
        platform = 'linux'

target = ARGUMENTS.get('target', 'template_release')
arch = ARGUMENTS.get('arch', 'universal' if platform == 'macos' else 'x86_64')
if arch not in ['x86_64', 'x86_32', 'arm64', 'universal']:
    print(f"ERROR: Invalid architecture '{arch}'. Supported architectures are: x86_64, x86_32, arm64, universal.")
    Exit(1)

# Set up the environment based on the platform
use_mingw_arg = ARGUMENTS.get('use_mingw', 'no')
use_mingw = use_mingw_arg.lower() in ['yes', 'true', '1']

if platform == 'windows' and not use_mingw:
    # Use the MSVC compiler on Windows (native build)
    env = Environment(tools=['default', 'msvc'])
elif platform == 'windows' and use_mingw:
    # Force SCons to use the MinGW toolchain for cross-compilation
    env = Environment(tools=['gcc', 'g++', 'gnulink', 'ar', 'gas'])
    
    # Explicitly override compiler settings after environment creation
    cc_cmd = os.environ.get('CC', 'x86_64-w64-mingw32-gcc')
    cxx_cmd = os.environ.get('CXX', 'x86_64-w64-mingw32-g++')
    
    env.Replace(CC=cc_cmd)
    env.Replace(CXX=cxx_cmd)
    env.Replace(LINK=cxx_cmd)  # Use C++ compiler for linking (g++) so libstdc++ and C++ EH symbols are pulled in

else:
    # Use the default compiler on other platforms
    env = Environment()

# Optional: enable SCons cache if SCONS_CACHE_DIR is provided (local or CI)
cache_dir = os.environ.get('SCONS_CACHE_DIR')
if cache_dir:
    CacheDir(cache_dir)

# Add include paths for godot-cpp and libtorrent
env.Append(CPPPATH=[
    'src',
    '.',
    'godot-cpp/include', 
    'godot-cpp/gen/include', 
    'godot-cpp/gdextension',
    'libtorrent/include',
])

# Platform-specific include paths - only add system paths for native builds
if not use_mingw and platform != 'windows':
    # Only add Linux/macOS system paths for native builds
    env.Append(CPPPATH=[
        '/usr/include',  # For system libtorrent headers
        '/usr/include/libtorrent',  # Specific path for libtorrent headers
        '/usr/local/include', # For Boost headers
        '/usr/local/include/boost' # Specific path for Boost headers
    ])
elif use_mingw or platform == 'windows':
    # For Windows cross-compilation, add MinGW-specific paths
    env.Append(CPPPATH=[
        '/usr/x86_64-w64-mingw32/include',  # MinGW system headers
        '/usr/x86_64-w64-mingw32/include/boost'  # MinGW Boost headers
    ])
    # Ensure MinGW uses its own sysroot and headers
    env.Append(CCFLAGS=['--sysroot=/usr/x86_64-w64-mingw32'])
    env.Append(LINKFLAGS=['--sysroot=/usr/x86_64-w64-mingw32'])

if platform == 'macos':
    # Add Homebrew OpenSSL lib path for macOS
    env.Append(LIBPATH=['/usr/local/opt/openssl@3/lib']) # Assuming openssl@3 is installed via Homebrew

env.Append(LIBPATH=['godot-cpp/bin', 'libtorrent/build'])

is_windows = platform == 'windows'
if is_windows and not use_mingw:
    # MSVC flags
    env.Append(CXXFLAGS=['/std:c++17'])
    env.Append(CPPDEFINES=['TORRENT_USE_OPENSSL'])
elif is_windows and use_mingw:
    # MinGW flags (similar to Linux but for Windows target)
    # Disable OpenSSL for Windows cross-compilation to avoid linking issues
    env.Append(CCFLAGS=['-fPIC'])
    env.Append(CXXFLAGS=['-std=c++17'])
    # Add Windows-specific defines for MinGW (removed TORRENT_USE_OPENSSL)
    env.Append(CPPDEFINES=['WIN32', '_WIN32', 'WINDOWS_ENABLED'])
    # Match godot-cpp's MinGW linking configuration for compatibility
    env.Append(CCFLAGS=['-Wwrite-strings'])
    env.Append(LINKFLAGS=['-Wl,--no-undefined'])
    # Use static linking to match godot-cpp's default use_static_cpp=True behavior
    env.Append(LINKFLAGS=['-static', '-static-libgcc', '-static-libstdc++'])
else:
    # Linux/macOS flags
    env.Append(CCFLAGS=['-fPIC'])
    env.Append(CXXFLAGS=['-std=c++17'])
    env.Append(CPPDEFINES=['TORRENT_USE_OPENSSL'])

# When using MinGW for cross-compilation, we still get .a files with lib prefix
# .lib files without prefix are only used with MSVC
if is_windows and not use_mingw:
    lib_ext = '.lib'
    lib_prefix = ''
else:
    lib_ext = '.a'
    lib_prefix = 'lib'

# Add godot-cpp library
godot_cpp_lib = f"{lib_prefix}godot-cpp.{platform}.{target}.{arch}{lib_ext}"
env.Append(LIBS=[File(os.path.join('godot-cpp', 'bin', godot_cpp_lib))])

# Libtorrent library linking - requires real libtorrent
# Use static library for all platforms to avoid runtime DLL dependencies
libtorrent_lib_path = os.path.join('libtorrent', 'build', 'libtorrent-rasterbar.a')

if os.path.exists(libtorrent_lib_path):
    env.Append(LIBS=[File(libtorrent_lib_path)])
    print("Using libtorrent library:", libtorrent_lib_path)
elif use_mingw or platform == 'windows':
    # When cross-compiling for Windows, don't try system detection
    # as it won't work on the Linux host
    print("ERROR: No libtorrent library found for Windows cross-compilation!")
    print("This is a pure wrapper - real libtorrent is required.")
    print("The libtorrent cache may not have been restored properly.")
    print("Expected library at:", libtorrent_lib_path)
    Exit(1)
else:
    # Try to find system libtorrent (Linux/macOS native builds only)
    conf = Configure(env)
    if conf.CheckLib('torrent-rasterbar'):
        env.Append(LIBS=['torrent-rasterbar'])
        print("Using system libtorrent-rasterbar")
    else:
        print("ERROR: No libtorrent library found!")
        print("This is a pure wrapper - real libtorrent is required.")
        print("Install Boost and run './build_local.sh linux' to build libtorrent.")
        Exit(1)
    env = conf.Finish()

if is_windows:
    # Windows libraries - removed SSL libraries to avoid MinGW linking issues
    env.Append(LIBS=['ws2_32', 'wsock32', 'iphlpapi', 'crypt32'])
elif platform == 'linux':
    env.Append(LIBS=['pthread', 'ssl', 'crypto', 'dl'])
elif platform == 'macos':
    env.Append(LIBS=['pthread', 'ssl', 'crypto']) # Added ssl and crypto for OpenSSL linking
    env.Append(FRAMEWORKS=['CoreFoundation', 'SystemConfiguration'])

# Debug logging for CI: print resolved names and compiler locations
print("=== SCons debug: resolved build variables ===")
print("platform:", platform)
print("target:", target)
print("arch:", arch)
print("use_mingw:", ARGUMENTS.get('use_mingw', 'no'))
print("expected godot_cpp_lib:", godot_cpp_lib)
print("godot-cpp bin path:", os.path.join('godot-cpp', 'bin'))
print("libtorrent build path:", os.path.join('libtorrent', 'build'))
print("libtorrent lib exists:", os.path.exists(os.path.join('libtorrent', 'build', 'libtorrent-rasterbar.a')))
print("ENV CC:", os.environ.get('CC'))
print("ENV CXX:", os.environ.get('CXX'))
print("env['CC']:", env.get('CC'))
print("env['CXX']:", env.get('CXX'))
print("============================================")

src_files = [
    'src/register_types.cpp',
    'src/torrent_error.cpp',
    'src/torrent_result.cpp',
    'src/torrent_logger.cpp',
    'src/torrent_session.cpp',
    'src/torrent_handle.cpp',
    'src/torrent_info.cpp',
    'src/torrent_status.cpp',
    'src/peer_info.cpp',
    'src/alert_manager.cpp',
]

env.Execute(Mkdir('addons/godot-torrent/bin'))

# Set the correct library suffix and prefix based on platform
if is_windows:
    env['SHLIBPREFIX'] = 'lib'
    env['SHLIBSUFFIX'] = '.dll'
elif platform == 'macos':
    env['SHLIBPREFIX'] = 'lib'
    env['SHLIBSUFFIX'] = '.dylib'
else:
    env['SHLIBPREFIX'] = 'lib'
    env['SHLIBSUFFIX'] = '.so'

# Debug logging: shared lib name details
print("SHLIBPREFIX:", env.get('SHLIBPREFIX'))
print("SHLIBSUFFIX:", env.get('SHLIBSUFFIX'))
print("Target shared lib will be created as:", env.get('SHLIBPREFIX') + 'godot-torrent' + env.get('SHLIBSUFFIX'))

# Create the library with a simple name, SCons will add the correct extension
library = env.SharedLibrary(target='libgodot-torrent', source=src_files)
installed_library = env.Install('addons/godot-torrent/bin', library)
Default(installed_library)