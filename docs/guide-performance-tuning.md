# Performance Tuning Guide

Optimize Godot-Torrent for maximum speed and efficiency.

## Quick Wins

### 1. Maximize Download Speed

```gdscript
# Remove all limits
session.set_download_rate_limit(0)  # Unlimited
session.set_upload_rate_limit(0)    # Unlimited

# Increase connections
session.set_max_connections(300)
session.set_max_uploads(8)

# Enable all peer discovery methods
session.start_dht()
```

### 2. Minimize Resource Usage

```gdscript
# Limit connections
session.set_max_connections(50)
session.set_max_uploads(2)

# Set modest rate limits
session.set_download_rate_limit(1024 * 1024)  # 1 MB/s
session.set_upload_rate_limit(256 * 1024)     # 256 KB/s

# Update UI less frequently
var update_interval = 2.0  # Seconds
```

### 3. Balance Performance and Resources

```gdscript
# Reasonable limits
session.set_max_connections(150)
session.set_max_uploads(4)
session.set_download_rate_limit(5 * 1024 * 1024)  # 5 MB/s
session.set_upload_rate_limit(1 * 1024 * 1024)    # 1 MB/s
```

## Network Optimization

### Port Forwarding

Forward your port in your router for better connectivity:

```gdscript
session.set_listen_port(6881)
# Then configure port forwarding: External 6881 -> Internal 6881
```

### DHT Configuration

```gdscript
# Enable DHT
session.start_dht()

# Add bootstrap nodes
session.add_dht_node("router.bittorrent.com", 6881)
session.add_dht_node("dht.transmissionbt.com", 6881)
```

### IPv6 Support

```gdscript
var settings = {
    "listen_interfaces": "0.0.0.0:6881,[::]:6881",  # IPv4 + IPv6
    "enable_dht": true
}
session.start_session_with_settings(settings)
```

## Download Optimization

### Priority Management

```gdscript
# Download important files first
var info = handle.get_torrent_info()
for i in range(info.get_file_count()):
    var path = info.get_file_path_at(i)
    if path.ends_with(".exe") or path.ends_with(".mp4"):
        handle.set_file_priority(i, 7)  # High priority
    else:
        handle.set_file_priority(i, 4)  # Normal
```

### Sequential Download (for Streaming)

```gdscript
# Download pieces in order
var piece_count = handle.get_torrent_info().get_piece_count()
for i in range(min(20, piece_count)):
    handle.set_piece_priority(i, 7)
```

### Skip Unwanted Files

```gdscript
# Don't download sample/extra files
for i in range(info.get_file_count()):
    var path = info.get_file_path_at(i).to_lower()
    if "sample" in path or path.ends_with(".nfo"):
        handle.set_file_priority(i, 0)  # Skip
```

## UI Performance

### Use Alert-Based Status Updates

**Always use the alert system instead of blocking calls:**

```gdscript
# ❌ BAD: Blocking, expensive
var status = handle.get_status()
var progress = status.get_progress()

# ✅ GOOD: Non-blocking, efficient
session.post_torrent_updates()
var alerts = session.get_alerts()

for alert in alerts:
    if alert.has("torrent_status"):
        for status_dict in alert["torrent_status"]:
            var progress = status_dict.get("progress", 0.0)
```

The alert system is:
- **Non-blocking**: Doesn't wait for libtorrent
- **Batched**: Gets status for all torrents at once
- **Efficient**: Optimized for frequent polling

### Update Less Frequently

```gdscript
var update_timer: float = 0.0
var update_interval: float = 1.0  # Update every second

func _process(delta):
    update_timer += delta
    if update_timer < update_interval:
        return

    update_timer = 0.0
    _update_ui()
```

### Batch Status Queries

```gdscript
# Bad: Query each torrent individually (blocking)
for handle in handles:
    var status = handle.get_status()  # Expensive and blocking!
    update_item(handle, status)

# Good: Use alerts to get all status updates at once (non-blocking)
func _process(_delta):
    # Request status updates for all torrents
    session.post_torrent_updates()
    
    # Get all alerts at once
    var alerts = session.get_alerts()
    
    for alert in alerts:
        if alert.has("torrent_status"):
            var statuses = alert["torrent_status"]
            for status_dict in statuses:
                var info_hash = status_dict.get("info_hash")
                if info_hash in torrent_map:
                    update_item(torrent_map[info_hash], status_dict)
```

### Use Object Pooling for UI

```gdscript
var torrent_item_pool: Array = []

func get_torrent_item():
    if torrent_item_pool.is_empty():
        return TorrentItem.instantiate()
    return torrent_item_pool.pop_back()

func return_torrent_item(item):
    item.visible = false
    torrent_item_pool.append(item)
```

## Memory Optimization

### Limit Active Torrents

```gdscript
var max_active_torrents = 5

func add_torrent(magnet):
    if active_torrents.size() >= max_active_torrents:
        # Queue or reject
        queued_torrents.append(magnet)
        return

    var handle = session.add_magnet_uri(magnet, download_path)
    active_torrents.append(handle)
```

### Clear Completed Torrents

```gdscript
func cleanup_completed():
    # Request status updates
    session.post_torrent_updates()
    var alerts = session.get_alerts()
    
    for alert in alerts:
        if alert.has("torrent_status"):
            for status_dict in alert["torrent_status"]:
                if status_dict.get("is_finished", false):
                    var info_hash = status_dict.get("info_hash")
                    var handle = find_handle_by_info_hash(info_hash)
                    
                    if handle:
                        # Save resume data
                        handle.save_resume_data()
                        
                        # Remove from session
                        session.remove_torrent(handle, false)
                        active_torrents.erase(handle)
```

## Bandwidth Management

### Fair Sharing

```gdscript
var total_download_limit = 10 * 1024 * 1024  # 10 MB/s total
var active_count = active_torrents.size()

if active_count > 0:
    var per_torrent_limit = total_download_limit / active_count
    for handle in active_torrents:
        # Note: Per-torrent limits not directly supported
        # Use session limits and manage active torrent count
        pass
```

### Time-Based Limits

```gdscript
func apply_time_based_limits():
    var hour = Time.get_time_dict_from_system()["hour"]

    # Daytime (9am - 5pm): Conservative
    if hour >= 9 and hour < 17:
        session.set_download_rate_limit(1 * 1024 * 1024)  # 1 MB/s
        session.set_upload_rate_limit(256 * 1024)         # 256 KB/s

    # Night time: Aggressive
    else:
        session.set_download_rate_limit(0)  # Unlimited
        session.set_upload_rate_limit(0)
```

## Disk I/O Optimization

### Flush Cache Strategically

```gdscript
# Flush cache periodically
var flush_timer = 0.0
var flush_interval = 60.0  # Every minute

func _process(delta):
    flush_timer += delta
    if flush_timer >= flush_interval:
        flush_timer = 0.0
        for handle in active_torrents:
            handle.flush_cache()
```

## Benchmarking

### Measure Performance

```gdscript
class_name PerformanceBenchmark

var start_time: int
var bytes_downloaded: int = 0
var bytes_uploaded: int = 0
var info_hash: String = ""

func start(handle: TorrentHandle):
    start_time = Time.get_ticks_msec()
    info_hash = handle.get_info_hash()

func update(session: TorrentSession):
    session.post_torrent_updates()
    var alerts = session.get_alerts()
    
    for alert in alerts:
        if alert.has("torrent_status"):
            for status_dict in alert["torrent_status"]:
                if status_dict.get("info_hash") == info_hash:
                    bytes_downloaded = status_dict.get("total_download", 0)
                    bytes_uploaded = status_dict.get("total_upload", 0)
                    return

func get_results() -> Dictionary:
    var elapsed = (Time.get_ticks_msec() - start_time) / 1000.0  # Seconds

    return {
        "duration": elapsed,
        "avg_download_speed": bytes_downloaded / elapsed,
        "avg_upload_speed": bytes_uploaded / elapsed,
        "total_downloaded": bytes_downloaded,
        "total_uploaded": bytes_uploaded
    }
```

## Performance Profiles

### Maximum Speed Profile

```gdscript
func apply_speed_profile():
    session.set_download_rate_limit(0)
    session.set_upload_rate_limit(0)
    session.set_max_connections(300)
    session.set_max_uploads(10)
    session.start_dht()

    print("⚡ Speed profile active")
```

### Battery Saver Profile

```gdscript
func apply_battery_profile():
    session.set_download_rate_limit(512 * 1024)  # 512 KB/s
    session.set_upload_rate_limit(128 * 1024)    # 128 KB/s
    session.set_max_connections(30)
    session.set_max_uploads(2)

    # Update less frequently
    update_interval = 5.0

    print("🔋 Battery saver active")
```

### Balanced Profile

```gdscript
func apply_balanced_profile():
    session.set_download_rate_limit(5 * 1024 * 1024)  # 5 MB/s
    session.set_upload_rate_limit(1 * 1024 * 1024)    # 1 MB/s
    session.set_max_connections(150)
    session.set_max_uploads(4)
    session.start_dht()

    print("⚖️ Balanced profile active")
```

## Troubleshooting Performance Issues

### Slow Downloads

1. **Check connection count**: Increase `set_max_connections()`
2. **Remove rate limits**: Set to 0 for unlimited
3. **Enable DHT**: `session.start_dht()`
4. **Forward ports**: Configure router
5. **Check torrent health**: Low peer count = slow

### High CPU Usage

1. **Reduce connection count**: Lower `set_max_connections()`
2. **Update UI less frequently**: Increase `update_interval`
3. **Limit active torrents**: Pause some torrents
4. **Disable verbose logging**: Use `TorrentLogger.WARNING`

### High Memory Usage

1. **Limit active torrents**: Queue excess torrents
2. **Clear completed torrents**: Remove from session
3. **Reduce connection count**: Lower limits
4. **Pool UI objects**: Reuse instead of recreate

---

**See also:**
- [Advanced Configuration](guide-advanced-config.md)
- [Troubleshooting](guide-troubleshooting.md)
