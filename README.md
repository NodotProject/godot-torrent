# Godot-Torrent

A comprehensive BitTorrent GDExtension for Godot 4, providing full protocol functionality with native C++ performance.

[![Build Status](https://github.com/NodotProject/godot-torrent/workflows/Build%20and%20Test/badge.svg)](https://github.com/NodotProject/godot-torrent/actions)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Godot](https://img.shields.io/badge/godot-4.1+-blue.svg)](https://godotengine.org/)

## Overview

Godot-Torrent wraps the powerful [libtorrent](https://libtorrent.org/) library to bring full-featured BitTorrent capabilities to Godot 4. Download, upload, and manage torrents directly from your Godot projects with native performance.

## Features

- **Complete BitTorrent Protocol** - Full support for downloading and seeding
- **Session Management** - Fine-grained control over torrent sessions
- **Real-time Monitoring** - Track progress, peers, and statistics
- **Priority Management** - Control file and piece download priorities
- **DHT Support** - Distributed Hash Table for trackerless torrents
- **Cross-Platform** - Native builds for Linux, Windows, and macOS
- **Production Ready** - Comprehensive error handling and logging

## Quick Start

```gdscript
# Create a session
var session = TorrentSession.new()
session.start_session()

# Enable DHT for magnet link support
session.start_dht()

# Add a torrent
var handle = session.add_magnet_uri(
    "magnet:?xt=urn:btih:...",
    "/path/to/downloads"
)

# Monitor progress (call this periodically, e.g., in a timer)
func update_progress():
    session.post_torrent_updates()
    var alerts = session.get_alerts()
    
    for alert in alerts:
        if alert.has("torrent_status"):
            for status_dict in alert["torrent_status"]:
                var progress = status_dict.get("progress", 0.0) * 100.0
                var download_rate = status_dict.get("download_rate", 0) / 1024.0 / 1024.0
                print("Progress: %.1f%% | Download: %.2f MB/s" % [progress, download_rate])
```

## Installation

### Prerequisites
- Godot 4.1+
- Git (for submodules)
- Build tools: GCC/Clang, SCons, CMake

### Build from Source

```bash
# Clone with submodules
git clone --recursive https://github.com/NodotProject/godot-torrent.git
cd godot-torrent

# Build for your platform
./build_local.sh linux    # Linux
./build_local.sh windows  # Windows cross-compile
./build_local.sh macos    # macOS

# Run tests
./run_tests.sh
```

For detailed build instructions, see [Building from Source](docs/BUILDING_FROM_SOURCE.md).

## Documentation

Full documentation is available in the [docs](docs/) directory:

- **[Getting Started](docs/GETTING_STARTED.md)** - Quick setup guide
- **[API Reference](docs/API_REFERENCE.md)** - Complete API documentation
- **[Tutorials](docs/TUTORIAL_BASIC_DOWNLOAD.md)** - Step-by-step guides
- **[Error Handling](docs/ERROR_HANDLING.md)** - Error codes and recovery
- **[Documentation Index](docs/index.md)** - Full documentation site

## Core Classes

- **TorrentSession** - Session management and configuration (30+ methods)
- **TorrentHandle** - Individual torrent control (30+ methods)
- **TorrentInfo** - Torrent metadata and file information (15+ methods)
- **TorrentStatus** - Real-time status tracking (25+ properties)
- **PeerInfo** - Peer connection details (20+ methods)
- **AlertManager** - Event system management
- **TorrentError** - Complete error handling system
- **TorrentLogger** - Comprehensive logging (5 levels, 10 categories)

## Use Cases

- **Game Development** - P2P content distribution, updates, mod sharing
- **Application Development** - File sharing, content delivery, backups
- **Media Applications** - Large file transfers, streaming content

See [Use Cases](docs/USE_CASES.md) for detailed examples.

## Performance

- **Build Time**: < 2 minutes with caching
- **Library Size**: 1.4MB optimized
- **Startup Time**: < 100ms session initialization
- **Memory Usage**: < 5MB base overhead
- **Network**: Full libtorrent throughput with DHT, UPnP, IPv6

## Testing

The project includes comprehensive testing using [GUT](https://github.com/bitwes/Gut):

```bash
# Run all tests
./run_tests.sh

# Run specific test categories
godot --headless -s addons/gut/gut_cmdln.gd -gdir=test/unit -gexit
godot --headless -s addons/gut/gut_cmdln.gd -gdir=test/integration -gexit
```

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Submit a pull request

## License

This project is licensed under the MIT License - see [LICENSE](LICENSE) for details.

## Acknowledgments

- **[libtorrent](https://libtorrent.org/)** - Powerful C++ BitTorrent library
- **[Godot Engine](https://godotengine.org/)** - Amazing open-source game engine
- **[GUT](https://github.com/bitwes/Gut)** - Godot Unit Testing framework
- **The Godot Community** - Continuous support and examples

## Support

- **Documentation**: [docs/](docs/)
- **Issues**: [GitHub Issues](https://github.com/NodotProject/godot-torrent/issues)
- **Discussions**: [GitHub Discussions](https://github.com/NodotProject/godot-torrent/discussions)

---

**Made with ❤️ for the Godot community**
