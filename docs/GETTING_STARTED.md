# Getting Started with Godot-Torrent

Welcome to Godot-Torrent! This guide will help you get up and running with BitTorrent functionality in your Godot project.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Installation](#installation)
3. [Your First Torrent Session](#your-first-torrent-session)
4. [Basic Concepts](#basic-concepts)
5. [Next Steps](#next-steps)

---

## Prerequisites

Before you begin, make sure you have:

- **Godot 4.1 or later** installed ([Download here](https://godotengine.org/download))
- Basic knowledge of GDScript
- Understanding of Godot's node system

### Optional (for building from source):
- Git (for cloning the repository)
- Build tools (see [BUILDING_FROM_SOURCE.md](BUILDING_FROM_SOURCE.md))

---

## Installation

### Option 1: Using Pre-built Binaries (Recommended)

1. Download the latest release from the [Releases page](https://github.com/NodotProject/godot-torrent/releases)
2. Extract the `addons/godot-torrent` folder to your project's `addons` directory
3. Restart Godot to load the extension

### Option 2: Building from Source

See [BUILDING_FROM_SOURCE.md](BUILDING_FROM_SOURCE.md) for detailed instructions.

### Verifying Installation

To verify the extension loaded correctly:

```gdscript
# In any script
extends Node

func _ready():
    var session = TorrentSession.new()
    if session:
        print("✅ Godot-Torrent loaded successfully!")
    else:
        print("❌ Failed to load Godot-Torrent")
```

---

## Your First Torrent Session

Let's create a simple script that downloads a torrent. This example uses a legal, public domain torrent for demonstration.

### Step 1: Create a New Scene

1. Create a new `Node` in your scene
2. Attach a new script to it
3. Copy the following code:

```gdscript
extends Node

# Reference to the torrent session
var session: TorrentSession
var handle: TorrentHandle

# Public domain sample torrent (Sintel movie trailer)
var magnet_uri = "magnet:?xt=urn:btih:08ada5a7a6183aae1e09d831df6748d566095a10&dn=Sintel&tr=udp%3A%2F%2Fexplodie.org%3A6969&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969&tr=udp%3A%2F%2Ftracker.empire-js.us%3A1337&tr=udp%3A%2F%2Ftracker.leechers-paradise.org%3A6969&tr=udp%3A%2F%2Ftracker.opentrackr.org%3A1337&tr=wss%3A%2F%2Ftracker.btorrent.xyz&tr=wss%3A%2F%2Ftracker.fastcast.nz&tr=wss%3A%2F%2Ftracker.openwebtorrent.com"

func _ready():
    # Create and start the session
    session = TorrentSession.new()

    if not session.start_session():
        push_error("Failed to start torrent session")
        return

    # Enable DHT for better peer discovery
    session.start_dht()

    print("🚀 Torrent session started!")

    # Add the torrent
    handle = session.add_magnet_uri(magnet_uri, "user://downloads")

    if handle and handle.is_valid():
        print("✅ Torrent added successfully!")
    else:
        push_error("Failed to add torrent")

func _process(_delta):
    # Update download progress every frame
    if handle and handle.is_valid():
        var status = handle.get_status()
        if status:
            var progress = status.get_progress() * 100.0
            var download_rate = status.get_download_rate() / 1024.0  # KB/s
            var num_peers = status.get_num_peers()

            # Print progress (every 60 frames = ~once per second)
            if Engine.get_process_frames() % 60 == 0:
                print("📥 Progress: %.1f%% | ⬇ %.1f KB/s | 👥 %d peers" % [progress, download_rate, num_peers])

            # Check if download is complete
            if status.is_finished():
                print("✨ Download complete!")
                set_process(false)  # Stop updating

func _exit_tree():
    # Clean up when the node is removed
    if session:
        session.stop_session()
    print("👋 Session stopped")
```

### Step 2: Run the Scene

1. Save the scene
2. Press F5 to run it
3. Watch the console output to see the download progress!

### What's Happening?

1. **Session Creation**: `TorrentSession.new()` creates a new torrent session
2. **Session Start**: `start_session()` initializes the BitTorrent engine
3. **DHT Enabled**: `start_dht()` enables Distributed Hash Table for peer discovery
4. **Add Torrent**: `add_magnet_uri()` adds a magnet link and starts downloading
5. **Monitor Progress**: `_process()` updates download statistics every frame
6. **Cleanup**: `_exit_tree()` stops the session when the node is removed

---

## Basic Concepts

### TorrentSession

The **TorrentSession** is the core class that manages all torrent operations. Think of it as your connection to the BitTorrent network.

```gdscript
var session = TorrentSession.new()
session.start_session()  # Start the engine
session.start_dht()      # Enable peer discovery
```

**Key methods:**
- `start_session()` - Initialize the session
- `stop_session()` - Shut down the session
- `add_magnet_uri()` - Add a torrent via magnet link
- `add_torrent_file()` - Add a torrent via .torrent file
- `start_dht()` - Enable DHT for trackerless torrents

### TorrentHandle

A **TorrentHandle** represents an individual torrent. You get a handle when you add a torrent.

```gdscript
var handle = session.add_magnet_uri(magnet, "downloads")

# Control the torrent
handle.pause()   # Pause download
handle.resume()  # Resume download
```

**Key methods:**
- `is_valid()` - Check if handle is still valid
- `pause()` / `resume()` - Control download state
- `get_status()` - Get current download status
- `get_torrent_info()` - Get torrent metadata

### TorrentStatus

**TorrentStatus** provides real-time information about a torrent's state.

```gdscript
var status = handle.get_status()

print("Progress: ", status.get_progress())       # 0.0 to 1.0
print("Download rate: ", status.get_download_rate())  # bytes/sec
print("Upload rate: ", status.get_upload_rate())      # bytes/sec
print("Peers: ", status.get_num_peers())              # peer count
print("State: ", status.get_state_string())           # "downloading", "seeding", etc.
```

**Key properties:**
- `get_progress()` - Download progress (0.0 - 1.0)
- `get_download_rate()` - Current download speed (bytes/sec)
- `get_upload_rate()` - Current upload speed (bytes/sec)
- `get_num_peers()` - Number of connected peers
- `is_finished()` - Check if download is complete

### TorrentInfo

**TorrentInfo** contains metadata about the torrent (files, size, etc.).

```gdscript
var info = handle.get_torrent_info()

if info and info.is_valid():
    print("Name: ", info.get_name())
    print("Total size: ", info.get_total_size(), " bytes")
    print("File count: ", info.get_file_count())
```

---

## Next Steps

Now that you have the basics down, explore more advanced features:

### Tutorials
- **[Basic Download Tutorial](TUTORIAL_BASIC_DOWNLOAD.md)** - Complete download manager
- **[Seeding Tutorial](TUTORIAL_SEEDING.md)** - Share files with others
- **[Advanced Configuration](GUIDE_ADVANCED_CONFIG.md)** - Customize your setup

### Guides
- **[Performance Tuning](GUIDE_PERFORMANCE_TUNING.md)** - Optimize for speed
- **[Troubleshooting](GUIDE_TROUBLESHOOTING.md)** - Solve common problems

### Reference
- **[API Reference](API_REFERENCE.md)** - Complete API documentation
- **[Error Handling](ERROR_HANDLING.md)** - Handle errors gracefully
- **[Debugging Guide](DEBUGGING_GUIDE.md)** - Debug your torrent application

---

## Common Questions

### Q: Where are files downloaded?

By default, files are downloaded to the path you specify in `add_magnet_uri()` or `add_torrent_file()`.

```gdscript
# Downloads to user://downloads (Godot's user data directory)
handle = session.add_magnet_uri(magnet, "user://downloads")

# Downloads to an absolute path
handle = session.add_magnet_uri(magnet, "/home/user/downloads")
```

### Q: How do I know when a download is complete?

Check the status:

```gdscript
var status = handle.get_status()
if status and status.is_finished():
    print("Download complete!")
```

### Q: Can I download multiple torrents at once?

Yes! Just add multiple torrents to the same session:

```gdscript
var handle1 = session.add_magnet_uri(magnet1, "downloads/torrent1")
var handle2 = session.add_magnet_uri(magnet2, "downloads/torrent2")
var handle3 = session.add_magnet_uri(magnet3, "downloads/torrent3")
```

### Q: How do I limit bandwidth usage?

Configure rate limits:

```gdscript
# Limit download to 1 MB/s
session.set_download_rate_limit(1 * 1024 * 1024)

# Limit upload to 512 KB/s
session.set_upload_rate_limit(512 * 1024)

# 0 = unlimited
session.set_download_rate_limit(0)
```

### Q: What's DHT and do I need it?

**DHT (Distributed Hash Table)** is a decentralized peer discovery system. It allows you to find peers without relying on trackers.

You should enable DHT for:
- Magnet links
- Trackerless torrents
- Better peer discovery

```gdscript
session.start_dht()  # Enable DHT
```

---

## Tips for Success

1. **Always check if handles are valid** before using them:
   ```gdscript
   if handle and handle.is_valid():
       # Safe to use handle
   ```

2. **Stop the session on exit** to clean up resources:
   ```gdscript
   func _exit_tree():
       if session:
           session.stop_session()
   ```

3. **Enable DHT for magnet links** - They won't work well without it:
   ```gdscript
   session.start_dht()
   ```

4. **Monitor progress in `_process()`** but don't update UI too frequently:
   ```gdscript
   if Engine.get_process_frames() % 60 == 0:  # Once per second
       update_ui()
   ```

5. **Use `user://` paths** for cross-platform compatibility:
   ```gdscript
   session.add_magnet_uri(magnet, "user://downloads")
   ```

---

## Example Project

Check out the `demo/` folder in the repository for complete working examples:

- `demo/test_basic.tscn` - Simple download example
- `demo/demo_advanced.tscn` - Advanced features demo

---

## Need Help?

- **Documentation**: [API Reference](API_REFERENCE.md)
- **Issues**: [GitHub Issues](https://github.com/NodotProject/godot-torrent/issues)
- **Discussions**: [GitHub Discussions](https://github.com/NodotProject/godot-torrent/discussions)

---

**🎉 Congratulations!** You're now ready to build torrent-powered applications with Godot!

Next: [Basic Download Tutorial →](TUTORIAL_BASIC_DOWNLOAD.md)
