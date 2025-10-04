# Advanced Configuration Guide

Master advanced Godot-Torrent features for optimal performance and control.

## Session Configuration

### Custom Settings on Start

```gdscript
var settings = {
    "user_agent": "MyApp/1.0",
    "listen_interfaces": "0.0.0.0:6881,[::]:6881",  # IPv4 + IPv6
    "enable_dht": true,
    "enable_lsd": true,
    "enable_upnp": true,
    "enable_natpmp": true,
    "download_rate_limit": 10 * 1024 * 1024,  # 10 MB/s
    "upload_rate_limit": 1 * 1024 * 1024,     # 1 MB/s
    "connections_limit": 200,
    "unchoke_slots_limit": 8
}

session.start_session_with_settings(settings)
```

### Network Configuration

```gdscript
# Port configuration
session.set_listen_port_range(6881, 6889)

# Bandwidth limits
session.set_download_rate_limit(5 * 1024 * 1024)  # 5 MB/s
session.set_upload_rate_limit(1 * 1024 * 1024)    # 1 MB/s

# Connection limits
session.set_max_connections(200)
session.set_max_uploads(4)
```

## DHT Configuration

```gdscript
# Start DHT
session.start_dht()

# Add bootstrap nodes
session.add_dht_node("router.bittorrent.com", 6881)
session.add_dht_node("dht.transmissionbt.com", 6881)
session.add_dht_node("router.utorrent.com", 6881)

# Save DHT state
var dht_state = session.save_dht_state()
var file = FileAccess.open("user://dht.dat", FileAccess.WRITE)
file.store_buffer(dht_state)
file.close()

# Load DHT state
var file2 = FileAccess.open("user://dht.dat", FileAccess.READ)
var loaded_state = file2.get_buffer(file2.get_length())
session.load_dht_state(loaded_state)
file2.close()
```

## Priority Management

### File Priorities

```gdscript
# Priority values: 0 = skip, 1-7 = low to high
handle.set_file_priority(0, 7)  # Download first file first
handle.set_file_priority(1, 0)  # Skip second file
handle.set_file_priority(2, 4)  # Normal priority

# Selective download
var info = handle.get_torrent_info()
for i in range(info.get_file_count()):
    var file_path = info.get_file_path_at(i)
    if file_path.ends_with(".mp4"):
        handle.set_file_priority(i, 7)  # High priority for videos
    else:
        handle.set_file_priority(i, 0)  # Skip others
```

### Piece Priorities

```gdscript
# Download first and last pieces first (for streaming)
var piece_count = info.get_piece_count()

handle.set_piece_priority(0, 7)  # First piece
handle.set_piece_priority(piece_count - 1, 7)  # Last piece

# Sequential download
for i in range(min(10, piece_count)):
    handle.set_piece_priority(i, 7)
```

## Resume Data Management

```gdscript
# Save resume data
handle.save_resume_data()

# Check alerts for resume data
func process_alerts():
    var alerts = session.get_alerts()
    for alert in alerts:
        if alert.get("type_name") == "save_resume_data_alert":
            var resume_data = alert.get("resume_data")
            save_resume_to_file(alert.get("info_hash"), resume_data)

# Load with resume data
var resume_data = load_resume_from_file(info_hash)
if resume_data.size() > 0:
    handle = session.add_magnet_uri_with_resume(magnet, path, resume_data)
```

## Session State Persistence

```gdscript
# Save session state on exit
func _exit_tree():
    if session:
        var state = session.save_state()
        var file = FileAccess.open("user://session.dat", FileAccess.WRITE)
        file.store_buffer(state)
        file.close()
        session.stop_session()

# Load session state on start
func _ready():
    session = TorrentSession.new()
    session.start_session()

    var file = FileAccess.open("user://session.dat", FileAccess.READ)
    if file:
        var state = file.get_buffer(file.get_length())
        session.load_state(state)
        file.close()
```

## Alert System

```gdscript
# Configure alert manager
var alert_manager = AlertManager.new()

# Enable specific categories
alert_manager.enable_error_alerts(true)
alert_manager.enable_status_alerts(true)
alert_manager.enable_tracker_alerts(true)
alert_manager.enable_peer_alerts(false)  # Can be noisy
alert_manager.enable_storage_alerts(true)
alert_manager.enable_dht_alerts(false)

# Process alerts
func process_alerts():
    var alerts = session.get_alerts()
    for alert in alerts:
        var type_name = alert.get("type_name")

        match type_name:
            "torrent_finished":
                on_torrent_finished(alert)
            "torrent_error":
                on_torrent_error(alert)
            "tracker_reply":
                on_tracker_reply(alert)
            "peer_connect":
                on_peer_connect(alert)

    session.clear_alerts()
```

## Tracker Management

```gdscript
# Add tracker
handle.add_tracker("http://tracker.example.com:80/announce", 0)

# Remove tracker
handle.remove_tracker("http://old.tracker.com:80/announce")

# Get all trackers
var trackers = handle.get_trackers()
for tracker in trackers:
    print("Tracker: %s (tier %d)" % [tracker["url"], tracker["tier"]])

# Force tracker announce
handle.force_reannounce()

# Scrape tracker
handle.scrape_tracker()
```

## Peer Management

```gdscript
# Get peer information
var peers = handle.get_peer_info()
for peer in peers:
    print("Peer: %s:%d" % [peer.get_ip(), peer.get_port()])
    print("  Client: %s" % peer.get_client())
    print("  Progress: %.1f%%" % (peer.get_progress() * 100.0))
    print("  Download: %d B/s" % peer.get_download_rate())
    print("  Upload: %d B/s" % peer.get_upload_rate())
    print("  Is seed: %s" % peer.is_seed())
```

## Advanced Operations

```gdscript
# Force recheck
handle.force_recheck()  # Verify all downloaded pieces

# Force DHT announce
handle.force_dht_announce()

# Move storage
handle.move_storage("/new/download/path")

# Flush cache
handle.flush_cache()

# Clear error
handle.clear_error()
```

## Performance Tuning

```gdscript
# For maximum download speed
session.set_download_rate_limit(0)  # Unlimited
session.set_max_connections(300)
session.set_max_uploads(8)
handle.set_file_priority(0, 7)  # High priority

# For minimum resource usage
session.set_max_connections(50)
session.set_max_uploads(2)
session.set_download_rate_limit(512 * 1024)  # 512 KB/s

# For seeding
session.set_upload_rate_limit(0)  # Unlimited upload
session.set_max_uploads(10)
session.set_max_connections(200)
```

## Logging Configuration

```gdscript
var logger = TorrentLogger.new()

# Enable logging
logger.enable_logging(true)

# Set log level
logger.set_log_level(TorrentLogger.DEBUG)

# Set log file
logger.set_log_file("user://torrent_debug.log")

# Attach to session
session.set_logger(logger)

# Log levels: NONE, ERROR, WARNING, INFO, DEBUG, TRACE
```

---

**See also:**
- [Performance Tuning Guide](guide-performance-tuning.md)
- [API Reference](api-reference.md)
