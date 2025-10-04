# Torrent Browser Example

Browse torrent contents and selectively download files using a tree view interface.

## Features

- Load torrents via magnet URI
- Browse file structure before downloading
- Selective file downloading with checkboxes
- File priority management
- Real-time download progress
- Metadata-only mode for browsing

## How to Run

1. Open the project in Godot 4
2. Navigate to `examples/torrent_browser/torrent_browser.tscn`
3. Press F5 or click "Run Scene"

## Usage

1. Enter a magnet URI
2. Click "Load Torrent" and wait for metadata
3. Browse the file list in the tree view
4. Uncheck files you don't want to download
5. Set the download path
6. Click "Start Download"
7. Monitor progress in real-time

## Code Highlights

### Loading Metadata Only
```gdscript
var handle = session.add_magnet_uri(magnet_uri, temp_path)
handle.pause()  # Only fetch metadata
```

### Browsing Files
```gdscript
var info = handle.get_torrent_info()
var file_count = info.get_file_count()
for i in range(file_count):
    var file_name = info.get_file_name(i)
    var file_size = info.get_file_size(i)
```

### Selective Download
```gdscript
# Set priority: 0 = skip, 4 = normal
handle.set_file_priority(file_index, priority)
handle.resume()  # Start download
```

## Learning Points

- Torrent metadata extraction
- File-level priority management
- UI tree view integration
- Selective downloading
- Pause/resume functionality
