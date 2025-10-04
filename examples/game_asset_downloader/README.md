# Game Asset Downloader Example

Download game assets, DLC, mods, and expansion packs with priority management.

## Features

- Browse available asset packs
- Download with priority management
- Track overall and per-file progress
- High-priority files download first
- Optimized settings for game assets
- Real-time download statistics
- Multiple connection support

## How to Run

1. Open the project in Godot 4
2. Navigate to `examples/game_asset_downloader/game_asset_downloader.tscn`
3. Press F5 or click "Run Scene"

## Usage

1. Select an asset pack from the list
2. Set the installation path
3. Click "Download Selected Pack"
4. Watch as high-priority files download first
5. Monitor per-file progress
6. Wait for download to complete

## Code Highlights

### Optimized Session Settings
```gdscript
var settings = {
    "enable_dht": true,
    "download_rate_limit": 10 * 1024 * 1024,  # 10 MB/s
    "connections_limit": 200,  # More connections
}
session.start_session_with_settings(settings)
```

### Priority-Based Downloading
```gdscript
# Essential files get highest priority
for file_idx in priority_files:
    handle.set_file_priority(file_idx, 7)  # Highest

# Other files get normal priority
handle.set_file_priority(other_idx, 4)  # Normal
```

### Per-File Progress Tracking
```gdscript
var info = handle.get_torrent_info()
for i in range(file_count):
    var file_name = info.get_file_name(i)
    var priority = handle.get_file_priority(i)
    # Create progress bar for each file
```

## Use Cases

### Game Updates
- Download critical files first (executables, core data)
- Background download of optional content
- Resume interrupted downloads

### DLC and Expansions
- Download only selected content
- Priority for campaign files over cosmetics
- Parallel downloads for faster acquisition

### Mod Distribution
- Community content delivery
- Large texture/model packs
- Automatic mod updates

## Learning Points

- File priority management
- Session optimization for large downloads
- Per-file progress tracking
- UI/UX for game asset downloading
- Connection limit configuration
- Real-world game integration patterns
