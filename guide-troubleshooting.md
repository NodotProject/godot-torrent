# Troubleshooting Guide

Solutions to common problems and issues with Godot-Torrent.

## Installation Issues

### GDExtension Not Loading

**Symptoms**: `TorrentSession` class not found

**Solutions**:
1. Verify `addons/godot-torrent/` folder exists
2. Check `godot-torrent.gdextension` file is present
3. Restart Godot editor
4. Check console for error messages

```bash
# Verify library file exists
ls addons/godot-torrent/bin/libgodot-torrent.so   # Linux
ls addons/godot-torrent/bin/libgodot-torrent.dll  # Windows
ls addons/godot-torrent/bin/libgodot-torrent.dylib # macOS
```

### Build Errors

**Symptoms**: Compilation fails

**Common causes**:
- Missing dependencies (Boost, OpenSSL)
- Wrong compiler version
- Outdated submodules

**Solutions**:
```bash
# Update submodules
git submodule update --init --recursive

# Install dependencies (Ubuntu/Debian)
sudo apt-get install libboost-all-dev libssl-dev cmake scons

# Clean and rebuild
rm -rf godot-cpp/bin libtorrent/build
./build_local.sh linux
```

See [building-from-source.md](building-from-source.md) for details.

---

## Connection Issues

### No Peers Connecting

**Symptoms**: `get_num_peers()` returns 0

**Solutions**:

1. **Enable DHT** (required for magnet links):
```gdscript
session.start_dht()
```

2. **Wait longer** - Peer discovery takes time (30-60 seconds)

3. **Check firewall**:
```bash
# Allow port in firewall (Linux)
sudo ufw allow 6881/tcp
sudo ufw allow 6881/udp
```

4. **Forward port** in router:
   - Router settings → Port Forwarding
   - External port: 6881
   - Internal port: 6881
   - Protocol: TCP + UDP

5. **Try different trackers**:
```gdscript
handle.add_tracker("udp://tracker.opentrackr.org:1337/announce", 0)
```

### Cannot Download Magnet Links

**Symptoms**: Magnet links added but nothing happens

**Solutions**:

1. **DHT must be enabled**:
```gdscript
session.start_dht()
```

2. **Wait for metadata** (can take 1-2 minutes):
```gdscript
func _process(_delta):
    var info = handle.get_torrent_info()
    if info and info.is_valid():
        print("Metadata received: ", info.get_name())
```

3. **Add bootstrap nodes**:
```gdscript
session.add_dht_node("router.bittorrent.com", 6881)
session.add_dht_node("dht.transmissionbt.com", 6881)
```

---

## Download Issues

### Download Not Starting

**Symptoms**: Progress stays at 0%

**Checks**:

1. **Verify handle is valid**:
```gdscript
if not handle or not handle.is_valid():
    push_error("Invalid handle")
```

2. **Check torrent state**:
```gdscript
var status = handle.get_status()
print("State: ", status.get_state_string())
# Should be "downloading" not "paused"
```

3. **Ensure not paused**:
```gdscript
if handle.is_paused():
    handle.resume()
```

4. **Check file priorities**:
```gdscript
var info = handle.get_torrent_info()
for i in range(info.get_file_count()):
    var priority = handle.get_file_priority(i)
    if priority == 0:
        print("File %d is skipped" % i)
```

### Slow Download Speeds

**Symptoms**: Low download rate, few peers

**Solutions**:

1. **Remove rate limits**:
```gdscript
session.set_download_rate_limit(0)  # Unlimited
```

2. **Increase connections**:
```gdscript
session.set_max_connections(200)
```

3. **Check peer count**:
```gdscript
var status = handle.get_status()
print("Peers: ", status.get_num_peers())
# Should be > 0, ideally 20+
```

4. **Verify torrent health** - Torrents with few seeders are naturally slow

5. **Forward your port** for better peer connectivity

### Download Stalls/Stops

**Symptoms**: Download stops progressing

**Solutions**:

1. **Check for errors**:
```gdscript
var alerts = session.get_alerts()
for alert in alerts:
    if alert.get("category") == "error":
        print("Error: ", alert.get("message"))
```

2. **Force recheck**:
```gdscript
handle.force_recheck()
```

3. **Verify disk space**:
```bash
df -h  # Check available space
```

4. **Check write permissions** on download directory

---

## Upload/Seeding Issues

### Not Uploading

**Symptoms**: Upload rate is 0

**Solutions**:

1. **Check if download is complete**:
```gdscript
var status = handle.get_status()
if not status.is_finished():
    print("Still downloading, limited upload")
```

2. **Remove upload limit**:
```gdscript
session.set_upload_rate_limit(0)
```

3. **Increase upload slots**:
```gdscript
session.set_max_uploads(8)
```

4. **Verify port forwarding** - Essential for uploading

5. **Wait for incoming connections** - Can take time

---

## Performance Issues

### High CPU Usage

**Symptoms**: Godot uses excessive CPU

**Solutions**:

1. **Update UI less frequently**:
```gdscript
var update_interval = 2.0  # Update every 2 seconds
```

2. **Reduce connection count**:
```gdscript
session.set_max_connections(50)
```

3. **Disable verbose logging**:
```gdscript
logger.set_log_level(TorrentLogger.WARNING)
```

4. **Limit active torrents**:
```gdscript
# Pause torrents when not needed
handle.pause()
```

### High Memory Usage

**Symptoms**: Memory usage grows over time

**Solutions**:

1. **Clear completed torrents**:
```gdscript
session.remove_torrent(handle, false)
```

2. **Limit active torrents**:
```gdscript
var max_active = 5
if active_torrents.size() > max_active:
    # Pause or remove some
```

3. **Clear alerts regularly**:
```gdscript
session.clear_alerts()
```

4. **Flush disk cache**:
```gdscript
handle.flush_cache()
```

---

## Error Messages

### "Failed to start session"

**Causes**:
- Port already in use
- Insufficient permissions
- Missing dependencies

**Solutions**:
1. Try different port range:
```gdscript
session.set_listen_port_range(6889, 6899)
```

2. Check port availability:
```bash
netstat -tuln | grep 6881
```

3. Run with proper permissions

### "Invalid torrent file"

**Causes**:
- Corrupted .torrent file
- Wrong file format
- Empty file data

**Solutions**:
1. Verify file:
```bash
file example.torrent  # Should say "BitTorrent file"
```

2. Re-download torrent file

3. Try different source

### "Save path cannot be empty"

**Cause**: Empty or invalid download path

**Solution**:
```gdscript
# Use valid path
var path = "user://downloads"
# or
var path = "/home/user/downloads"  # Absolute path

# Ensure directory exists
var dir = DirAccess.open("user://")
if not dir.dir_exists("downloads"):
    dir.make_dir("downloads")
```

---

## Debug Techniques

### Enable Detailed Logging

```gdscript
var logger = TorrentLogger.new()
logger.enable_logging(true)
logger.set_log_level(TorrentLogger.DEBUG)
logger.set_log_file("user://debug.log")
session.set_logger(logger)
```

### Monitor Alerts

```gdscript
func _process(_delta):
    var alerts = session.get_alerts()
    for alert in alerts:
        print("[%s] %s: %s" % [
            alert.get("category"),
            alert.get("type_name"),
            alert.get("message")
        ])
    session.clear_alerts()
```

### Check Handle Status

```gdscript
func debug_handle(handle: TorrentHandle):
    if not handle:
        print("Handle is null")
        return

    if not handle.is_valid():
        print("Handle is invalid")
        return

    var status = handle.get_status()
    if not status:
        print("Cannot get status")
        return

    print("=== Torrent Debug Info ===")
    print("Name: ", handle.get_name())
    print("State: ", status.get_state_string())
    print("Progress: %.2f%%" % (status.get_progress() * 100))
    print("Download rate: %d B/s" % status.get_download_rate())
    print("Upload rate: %d B/s" % status.get_upload_rate())
    print("Peers: %d" % status.get_num_peers())
    print("Seeds: %d" % status.get_num_seeds())
    print("Is paused: %s" % status.is_paused())
    print("Is finished: %s" % status.is_finished())
```

### Test with Known-Good Torrent

Use public domain test torrent:
```gdscript
# Sintel movie trailer (legal, well-seeded)
var test_magnet = "magnet:?xt=urn:btih:08ada5a7a6183aae1e09d831df6748d566095a10"
var handle = session.add_magnet_uri(test_magnet, "user://test")
```

---

## Getting Help

If you're still stuck:

1. **Check documentation**:
   - [API Reference](api-reference.md)
   - [Error Handling Guide](error-handling.md)
   - [Debugging Guide](debugging-guide.md)

2. **Search existing issues**:
   - [GitHub Issues](https://github.com/NodotProject/godot-torrent/issues)

3. **Create a bug report** with:
   - Godot version
   - Operating system
   - Steps to reproduce
   - Debug logs
   - Minimal code example

4. **Join discussions**:
   - [GitHub Discussions](https://github.com/NodotProject/godot-torrent/discussions)

---

## Common Gotchas

1. **Always check handle validity**:
```gdscript
if handle and handle.is_valid():
    # Safe to use
```

2. **Enable DHT for magnet links**:
```gdscript
session.start_dht()
```

3. **Stop session on exit**:
```gdscript
func _exit_tree():
    if session:
        session.stop_session()
```

4. **Wait for metadata** with magnet links - It takes time!

5. **Forward your port** for best connectivity

---

**Still need help?** See [Getting Started](getting-started.md) for basics or open an [issue](https://github.com/NodotProject/godot-torrent/issues).
