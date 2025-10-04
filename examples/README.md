# Godot-Torrent Examples

Complete example projects demonstrating the Godot-Torrent library in real-world scenarios.

## Overview

This directory contains 5 fully-functional example applications that showcase different use cases and features of the Godot-Torrent library. Each example is self-contained with its own scene, script, and documentation.

## Examples

### 1. Download Manager
**Path:** `download_manager/`
**Complexity:** Beginner
**Features:**
- Simple GUI for downloading torrents
- Real-time progress tracking
- Multiple simultaneous downloads
- Download/upload speed monitoring
- Peer/seed count display

**Best for:** Learning the basics of torrent downloading and session management.

[View Documentation](download_manager/README.md)

---

### 2. Torrent Browser
**Path:** `torrent_browser/`
**Complexity:** Intermediate
**Features:**
- Browse torrent contents before downloading
- Selective file downloading
- File priority management
- Tree view file browser
- Metadata-only loading

**Best for:** Understanding file-level control and priority management.

[View Documentation](torrent_browser/README.md)

---

### 3. P2P File Sharing
**Path:** `p2p_file_sharing/`
**Complexity:** Intermediate
**Features:**
- Create torrents from local files
- Generate magnet URIs
- DHT and Local Service Discovery
- Upload statistics tracking
- Clipboard integration

**Best for:** Learning about torrent creation and seeding.

[View Documentation](p2p_file_sharing/README.md)

---

### 4. Game Asset Downloader
**Path:** `game_asset_downloader/`
**Complexity:** Advanced
**Features:**
- Download DLC and expansion packs
- Priority-based file downloading
- Per-file progress tracking
- Asset pack catalog
- Optimized for large downloads

**Best for:** Real-world game development integration.

[View Documentation](game_asset_downloader/README.md)

---

### 5. Seeding Server
**Path:** `seeding_server/`
**Complexity:** Advanced
**Features:**
- Headless server operation
- JSON configuration
- Multiple torrent seeding
- Statistics dashboard
- Production-ready deployment

**Best for:** Setting up dedicated seeding infrastructure.

[View Documentation](seeding_server/README.md)

---

## Quick Start

### Running Examples in Godot Editor

1. Open the godot-torrent project in Godot 4
2. Navigate to any example's `.tscn` file
3. Press **F5** or click "Run Current Scene"

Example paths:
- `examples/download_manager/download_manager.tscn`
- `examples/torrent_browser/torrent_browser.tscn`
- `examples/p2p_file_sharing/p2p_file_sharing.tscn`
- `examples/game_asset_downloader/game_asset_downloader.tscn`
- `examples/seeding_server/seeding_server.tscn`

### Running Headless (Seeding Server)

```bash
# From project directory
godot --headless examples/seeding_server/seeding_server.tscn

# Or run the script directly
godot --headless --script examples/seeding_server/seeding_server.gd
```

## Learning Path

We recommend exploring the examples in this order:

1. **Download Manager** - Start here to learn basic session and torrent management
2. **Torrent Browser** - Learn about file-level control and priorities
3. **P2P File Sharing** - Understand torrent creation and seeding
4. **Game Asset Downloader** - See real-world game integration patterns
5. **Seeding Server** - Deploy production-ready infrastructure

## Common Patterns

### Session Initialization
```gdscript
var session = TorrentSession.new()
session.start_session()
```

### Adding a Torrent
```gdscript
var handle = session.add_magnet_uri(magnet_uri, download_path)
```

### Monitoring Progress
```gdscript
var status = handle.get_status()
var progress = status.get_progress() * 100.0
var download_rate = status.get_download_rate()
```

### File Priorities
```gdscript
handle.set_file_priority(file_index, priority)  # 0=skip, 7=high
```

## Use Cases by Industry

### Game Development
- **Download Manager**: Patch distribution
- **Torrent Browser**: Selective DLC installation
- **Game Asset Downloader**: Content updates and mods
- **Seeding Server**: Update server infrastructure

### Software Distribution
- **Download Manager**: Software downloads
- **P2P File Sharing**: Beta testing distribution
- **Seeding Server**: Release hosting

### Media & Content
- **Torrent Browser**: Large file previewing
- **P2P File Sharing**: Content creator distribution
- **Seeding Server**: Media library hosting

## Requirements

- **Godot:** 4.1 or higher
- **Platform:** Linux, Windows, or macOS
- **Network:** Internet connection for torrent operations
- **Storage:** Varies by example (minimal for demos)

## Testing the Examples

Most examples work with any valid magnet URI or .torrent file. For testing purposes, you can use:

1. **Legal Torrents:**
   - Linux distributions (Ubuntu, Debian, etc.)
   - Creative Commons content
   - Public domain materials

2. **Test Torrents:**
   - Create your own using the P2P File Sharing example
   - Use small open-source project releases

## Troubleshooting

### Example Won't Run
- Ensure the library is built: `./build_local.sh linux`
- Check that `libgodot-torrent.so` exists in the project root
- Verify Godot 4.1+ is installed

### No Peers Connecting
- Check firewall settings
- Enable UPnP in router
- Wait for DHT to bootstrap (30-60 seconds)
- Try torrents with known active seeders

### Slow Downloads
- Check bandwidth limits in session settings
- Verify internet connection speed
- Try torrents with more seeders
- Increase connection limits

## Contributing

Found a bug in an example? Have an idea for a new example? Please:

1. Open an issue on GitHub
2. Submit a pull request
3. Join the discussion

## License

All examples are licensed under MIT, same as the main project. Feel free to use them as starting points for your own projects.

## Support

- **Documentation:** [Main Docs](../docs/)
- **API Reference:** [API_REFERENCE.md](../docs/API_REFERENCE.md)
- **Issues:** [GitHub Issues](https://github.com/NodotProject/godot-torrent/issues)

---

**Happy torrenting with Godot!** 🎮⚡
