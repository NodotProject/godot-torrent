# P2P File Sharing Example

Create torrents from local files and share them with others via P2P.

## Features

- Select files to share
- Generate magnet URIs
- DHT and Local Service Discovery
- Track upload statistics
- Copy magnet links to clipboard
- Monitor connected peers
- Share multiple files simultaneously

## How to Run

1. Open the project in Godot 4
2. Navigate to `examples/p2p_file_sharing/p2p_file_sharing.tscn`
3. Press F5 or click "Run Scene"

## Usage

1. Click "Browse" to select a file to share
2. Optionally add a tracker URL (DHT is enabled by default)
3. Click "Share File"
4. Click on the shared file in the list to see its magnet URI
5. Click "Copy to Clipboard" to copy the magnet link
6. Share the magnet URI with others
7. Monitor upload speed and peer connections

## Code Highlights

### Enable DHT for Trackerless Sharing
```gdscript
var settings = {
    "enable_dht": true,
    "enable_lsd": true,
}
session.apply_settings(settings)
```

### Create Torrent (Conceptual)
```gdscript
# In full implementation with TorrentCreator:
var creator = TorrentCreator.new()
creator.add_file(file_path)
creator.set_tracker(tracker_url)
var torrent_data = creator.generate()
```

### Seed the Torrent
```gdscript
# Add in seed mode
var handle = session.add_torrent_file(torrent_data, file_directory)
```

### Generate Magnet URI
```gdscript
var magnet_uri = handle.make_magnet_uri()
```

## Learning Points

- Torrent creation workflow
- DHT and trackerless operation
- Seeding and upload monitoring
- Magnet URI generation
- P2P file distribution
- Local Service Discovery

## Note

This example demonstrates the workflow for P2P file sharing. The full implementation requires the TorrentCreator class (Phase 5 feature) for creating actual .torrent files.
