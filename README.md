# Godot-Torrent

<p align="center">
    <img width="256" height="256" alt="image" src="https://github.com/NodotProject/godot-torrent/blob/main/addons/godot-torrent/icon.png?raw=true" />
</p>

A comprehensive BitTorrent GDExtension for Godot 4, providing full protocol functionality with native C++ performance.

[![Discord](https://img.shields.io/discord/1089846386566111322)](https://discord.gg/Rx9CZX4sjG) [![Mastodon](https://img.shields.io/mastodon/follow/110106863700290562?domain=mastodon.gamedev.place)](https://mastodon.gamedev.place/@krazyjakee) [![Youtube](https://img.shields.io/youtube/channel/subscribers/UColWkNMgHseKyU7D1QGeoyQ)](https://www.youtube.com/@GodotNodot) [![GitHub Sponsors](https://img.shields.io/github/sponsors/krazyjakee)](https://github.com/sponsors/krazyjakee) [![GitHub Stars](https://img.shields.io/github/stars/NodotProject/godot-torrent)](https://github.com/NodotProject/godot-torrent)

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Quick Start](#quick-start)
- [Installation](#installation)
  - [Prerequisites](#prerequisites)
  - [Build from Source](#build-from-source)
- [Documentation](#documentation)
- [Core Classes](#core-classes)
- [Use Cases](#use-cases)
- [Performance](#performance)
- [Testing](#testing)
- [Contributing](#contributing)
- [Support Me](#-support-me)
- [License](#license)
- [Acknowledgments](#acknowledgments)

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
- Godot 4.5+
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

## 💖 Support Me
Hi! I’m krazyjakee 🎮, creator and maintain­er of the *NodotProject* - a suite of open‑source Godot tools (e.g. Nodot, Gedis, GedisQueue etc) that empower game developers to build faster and maintain cleaner code.

I’m looking for sponsors to help sustain and grow the project: more dev time, better docs, more features, and deeper community support. Your support means more stable, polished tools used by indie makers and studios alike.

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/krazyjakee)

Every contribution helps maintain and improve this project. And encourage me to make more projects like this!

*This is optional support. The tool remains free and open-source regardless.*

---

**Created with ❤️ for Godot Developers**  
For contributions, please open issues on GitHub
