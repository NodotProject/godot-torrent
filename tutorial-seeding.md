# Tutorial: Seeding Torrents

Learn how to share files with others through seeding.

## What is Seeding?

**Seeding** is when you share completed download files with other users. It's the backbone of the BitTorrent protocol - seeders keep torrents alive!

## Quick Start

```gdscript
extends Node

var session: TorrentSession
var handle: TorrentHandle

func _ready():
    session = TorrentSession.new()
    session.start_session()
    session.start_dht()

    # Configure for seeding
    session.set_upload_rate_limit(0)  # Unlimited upload
    session.set_max_uploads(8)        # Allow more upload slots

    # Add completed torrent
    var torrent_file = FileAccess.get_file_as_bytes("path/to/file.torrent")
    handle = session.add_torrent_file(torrent_file, "path/to/downloaded/files")

    # Torrent will automatically start seeding if files are complete
    print("🌱 Seeding started!")

func _process(_delta):
    if handle and handle.is_valid():
        var status = handle.get_status()
        if status and status.is_seeding():
            var upload_rate = status.get_upload_rate() / 1024.0
            var uploaded = status.get_total_upload() / (1024.0 * 1024.0)
            var peers = status.get_num_peers()

            print("📤 Upload: %.1f KB/s | Uploaded: %.1f MB | Peers: %d" % [
                upload_rate, uploaded, peers
            ])
```

## Seeding Configuration

### Optimal Settings for Seeders

```gdscript
func configure_for_seeding():
    # Bandwidth
    session.set_upload_rate_limit(0)  # Unlimited (or set your preferred limit)
    session.set_download_rate_limit(0)

    # Connections
    session.set_max_connections(200)  # Allow many connections
    session.set_max_uploads(8)        # More upload slots

    # DHT for better peer discovery
    session.start_dht()

    print("✅ Configured for optimal seeding")
```

### Setting Upload Ratio Goals

```gdscript
# Seed until you've uploaded 2x what you downloaded
var target_ratio: float = 2.0

func check_ratio():
    var status = handle.get_status()
    if not status:
        return

    var downloaded = status.get_total_download()
    var uploaded = status.get_total_upload()

    if downloaded > 0:
        var current_ratio = float(uploaded) / float(downloaded)

        if current_ratio >= target_ratio:
            print("✅ Target ratio reached: %.2f" % current_ratio)
            handle.pause()  # Stop seeding
        else:
            print("📊 Current ratio: %.2f / %.2f" % [current_ratio, target_ratio])
```

## Monitoring Seeding Activity

```gdscript
extends Control

@onready var upload_label: Label = $UploadStats
@onready var ratio_label: Label = $RatioStats
@onready var peers_label: Label = $PeerStats

func _process(_delta):
    if not handle or not handle.is_valid():
        return

    var status = handle.get_status()
    if not status:
        return

    # Upload stats
    var upload_rate = status.get_upload_rate() / 1024.0  # KB/s
    var total_uploaded = status.get_total_upload() / (1024.0 * 1024.0)  # MB
    upload_label.text = "Upload: %.1f KB/s (%.1f MB total)" % [upload_rate, total_uploaded]

    # Ratio
    var downloaded = status.get_total_download()
    var uploaded = status.get_total_upload()
    var ratio = float(uploaded) / float(downloaded) if downloaded > 0 else 0.0
    ratio_label.text = "Ratio: %.2f" % ratio

    # Peers
    var num_peers = status.get_num_peers()
    peers_label.text = "Connected peers: %d" % num_peers
```

## Best Practices

### 1. Seed What You Download

Always seed files you download to give back to the community:

```gdscript
func on_download_complete(torrent_handle: TorrentHandle):
    # Keep seeding after download completes
    print("✅ Download complete - continuing to seed")
    # Don't remove the torrent, let it seed!
```

### 2. Set Reasonable Upload Limits

Don't overwhelm your connection:

```gdscript
# Limit to 50% of your upload bandwidth
var upload_bandwidth = 1024 * 1024  # 1 MB/s total
var torrent_limit = upload_bandwidth / 2
session.set_upload_rate_limit(torrent_limit)
```

### 3. Use DHT

DHT helps peers find you even without trackers:

```gdscript
session.start_dht()
```

### 4. Open Your Ports

Forward your listening port for better connectivity:

```gdscript
session.set_listen_port(6881)
# Then forward port 6881 in your router
```

## Automatic Seeding Management

```gdscript
class_name SeedManager
extends Node

var session: TorrentSession
var seeding_torrents: Dictionary = {}

# Configuration
var max_seeding_torrents: int = 5
var seed_time_limit: int = 7 * 24 * 3600  # 7 days in seconds
var target_ratio: float = 2.0

func add_to_seed_queue(handle: TorrentHandle):
    seeding_torrents[handle] = {
        "start_time": Time.get_unix_time_from_system(),
        "initial_uploaded": handle.get_status().get_total_upload()
    }

func _process(_delta):
    _manage_seeding_slots()
    _check_seeding_goals()

func _manage_seeding_slots():
    # Limit number of active seeders
    var active_count = 0

    for handle in seeding_torrents.keys():
        if handle.is_valid():
            var status = handle.get_status()
            if status and status.is_seeding() and not status.is_paused():
                active_count += 1

    # If too many active, pause some
    if active_count > max_seeding_torrents:
        _pause_least_active_seeders(active_count - max_seeding_torrents)

func _pause_least_active_seeders(count: int):
    # Sort by upload rate, pause slowest
    var sorted_handles = []

    for handle in seeding_torrents.keys():
        if not handle.is_valid():
            continue

        var status = handle.get_status()
        if status and status.is_seeding():
            sorted_handles.append({
                "handle": handle,
                "upload_rate": status.get_upload_rate()
            })

    sorted_handles.sort_custom(func(a, b): return a.upload_rate < b.upload_rate)

    for i in range(min(count, sorted_handles.size())):
        sorted_handles[i].handle.pause()

func _check_seeding_goals():
    var current_time = Time.get_unix_time_from_system()

    for handle in seeding_torrents.keys():
        if not handle.is_valid():
            continue

        var data = seeding_torrents[handle]
        var status = handle.get_status()
        if not status:
            continue

        # Check time limit
        var elapsed = current_time - data.start_time
        if elapsed > seed_time_limit:
            print("⏰ Seed time limit reached for torrent")
            handle.pause()
            continue

        # Check ratio
        var downloaded = status.get_total_download()
        var uploaded = status.get_total_upload()
        if downloaded > 0:
            var ratio = float(uploaded) / float(downloaded)
            if ratio >= target_ratio:
                print("✅ Target ratio %.2f reached" % ratio)
                handle.pause()
```

## Troubleshooting

### No upload activity

1. **Check if port is forwarded** in your router
2. **Ensure DHT is enabled**: `session.start_dht()`
3. **Check upload limit**: `session.set_upload_rate_limit(0)`
4. **Verify files are complete**

### Low upload speeds

1. **Increase max uploads**: `session.set_max_uploads(8)`
2. **Remove upload limit**: `session.set_upload_rate_limit(0)`
3. **Wait for more peers to connect**

---

**Next**: [Advanced Configuration Guide →](guide-advanced-config.md)
