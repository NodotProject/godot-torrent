# Seeding Server Example

A headless seeding server for dedicated content distribution.

## Features

- Headless operation (no GUI)
- JSON configuration file
- Multiple torrent seeding
- Server-optimized settings
- Real-time statistics
- Automatic port mapping (UPnP/NAT-PMP)
- High connection limits
- Graceful shutdown

## How to Run

### From Godot Editor
1. Open the project in Godot 4
2. Navigate to `examples/seeding_server/seeding_server.tscn`
3. Press F5 or click "Run Scene"

### Headless Mode (Production)
```bash
# Run headless
godot --headless --script examples/seeding_server/seeding_server.gd

# Or export and run as executable
godot --export-release "Linux/X11" seeding_server.x86_64
./seeding_server.x86_64
```

## Configuration

On first run, a default configuration file is created at `user://seeding_server_config.json`.

### Example Configuration
```json
{
  "torrents": [
    {
      "type": "magnet",
      "uri": "magnet:?xt=urn:btih:...",
      "path": "/path/to/content"
    },
    {
      "type": "file",
      "path": "/path/to/file.torrent",
      "content_path": "/path/to/content"
    }
  ],
  "settings": {
    "upload_limit_mbps": 50,
    "max_connections": 500
  }
}
```

### Configuration Options

**Torrent Types:**
- `magnet`: Add torrent via magnet URI
- `file`: Add torrent from .torrent file

**Settings:**
- `upload_limit_mbps`: Maximum upload speed in MB/s
- `max_connections`: Maximum peer connections

## Usage

1. Edit the configuration file with your torrents
2. Restart the server
3. Monitor statistics in console output
4. Press Ctrl+C to gracefully shutdown

## Code Highlights

### Server-Optimized Settings
```gdscript
var settings = {
    "enable_dht": true,
    "enable_upnp": true,
    "upload_rate_limit": 50 * 1024 * 1024,  # 50 MB/s
    "connections_limit": 500,
    "unchoke_slots_limit": 50,  # Serve many peers
}
```

### Loading Torrents
```gdscript
# From magnet URI
var handle = session.add_magnet_uri(magnet_uri, content_path)

# From .torrent file
var torrent_data = FileAccess.get_file_as_bytes(torrent_path)
var handle = session.add_torrent_file(torrent_data, content_path)
```

### Statistics Tracking
```gdscript
var status = handle.get_status()
var upload_rate = status.get_upload_rate()
var num_peers = status.get_num_peers()
var total_uploaded = status.get_total_upload()
```

## Use Cases

### Content Distribution Networks
- Game update distribution
- Software releases
- Large file hosting
- Mirror services

### Community Seeding
- Open source software distribution
- Linux ISO hosting
- Archive preservation
- Dataset sharing

### Private Networks
- Corporate content delivery
- Internal software distribution
- Backup replication
- Media libraries

## Performance

**Recommended Hardware:**
- CPU: 2+ cores
- RAM: 2GB minimum
- Network: Symmetric connection recommended
- Storage: SSD for better I/O

**Scaling:**
- Each torrent: ~10-20 MB RAM
- 500 peers: ~500 MB RAM
- Disk I/O is the main bottleneck

## Learning Points

- Headless Godot operation
- Server configuration management
- JSON file handling
- Statistics collection
- Graceful shutdown handling
- Production deployment patterns
- Connection and bandwidth management
