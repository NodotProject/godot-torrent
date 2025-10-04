# Download Manager Example

A simple download manager demonstrating basic torrent downloading with a GUI interface.

## Features

- Add torrents via magnet URI
- Real-time progress tracking
- Display download/upload rates
- Show peer and seed counts
- Display torrent state
- Multiple simultaneous downloads

## How to Run

1. Open the project in Godot 4
2. Navigate to `examples/download_manager/download_manager.tscn`
3. Press F5 or click "Run Scene"

## Usage

1. Enter a magnet URI in the "Magnet URI" field
2. Optionally change the download path (defaults to user data directory)
3. Click "Add Torrent"
4. Watch the download progress in the list
5. Click on a torrent to see detailed statistics

## Code Highlights

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

## Learning Points

- Basic session management
- Magnet URI handling
- Real-time status updates using timers
- UI integration with Godot's Control nodes
- Multiple torrent management with dictionaries
