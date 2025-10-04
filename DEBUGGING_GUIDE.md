# Debugging Guide

## Overview

This guide covers logging, debugging techniques, and troubleshooting for the godot-torrent library.

## Logging System

### Quick Start

```gdscript
extends Node

var session: TorrentSession
var logger: TorrentLogger

func _ready():
    # Create logger
    logger = TorrentLogger.new()
    logger.enable_logging(true)
    logger.set_log_level(TorrentLogger.INFO)

    # Attach logger to session
    session = TorrentSession.new()
    session.set_logger(logger)
    session.start_session()
```

### Log Levels

The logging system provides five log levels, each including all levels below it:

| Level | Value | Description | Use Case |
|-------|-------|-------------|----------|
| `NONE` | 0 | Logging disabled | Production (no logging overhead) |
| `ERROR` | 1 | Errors only | Production (critical issues only) |
| `WARNING` | 2 | Warnings + Errors | Production (potential issues) |
| `INFO` | 3 | Info + Warning + Error | Development (general information) |
| `DEBUG` | 4 | Debug + above | Development (detailed debugging) |
| `TRACE` | 5 | Trace + above | Development (verbose logging) |

```gdscript
# Set log level
logger.set_log_level(TorrentLogger.DEBUG)

# Or via session
session.set_log_level(TorrentLogger.DEBUG)
```

### Log Categories

Filter logs by category for focused debugging:

| Category | Description |
|----------|-------------|
| `ALL` | All categories (default) |
| `SESSION` | Session lifecycle and configuration |
| `TORRENT` | Torrent operations |
| `PEER` | Peer connections and communication |
| `TRACKER` | Tracker announces and responses |
| `DHT` | DHT operations |
| `PORT_MAPPING` | UPnP/NAT-PMP port mapping |
| `STORAGE` | File storage operations |
| `PERFORMANCE` | Performance metrics |
| `ALERT` | Libtorrent alerts |

```gdscript
# Enable specific categories only
logger.disable_all_categories()
logger.enable_category(TorrentLogger.TORRENT, true)
logger.enable_category(TorrentLogger.TRACKER, true)

# Or enable all (default)
logger.enable_all_categories()
```

### File Logging

Save logs to a file for later analysis:

```gdscript
# Enable file logging
logger.set_log_file("user://torrent_debug.log")

# Logs are written to both console and file

# Close file when done
logger.close_log_file()
```

Log file location:
- **Windows**: `%APPDATA%\Godot\app_userdata\<project_name>\torrent_debug.log`
- **Linux**: `~/.local/share/godot/app_userdata/<project_name>/torrent_debug.log`
- **macOS**: `~/Library/Application Support/Godot/app_userdata/<project_name>/torrent_debug.log`

### Manual Logging

Log custom messages at any level:

```gdscript
logger.log_error("Critical error occurred", "CUSTOM")
logger.log_warning("Potential issue detected", "CUSTOM")
logger.log_info("Download started", "CUSTOM")
logger.log_debug("Piece verified", "CUSTOM")
logger.log_trace("Detailed trace info", "CUSTOM")

# Or use generic log method
logger.log(TorrentLogger.INFO, "Custom message", "CATEGORY")
```

### Log Statistics

Monitor logging activity:

```gdscript
var stats = logger.get_log_stats()
print("Errors: ", stats["error_count"])
print("Warnings: ", stats["warning_count"])
print("Info: ", stats["info_count"])
print("Total: ", stats["total_count"])

# Reset statistics
logger.reset_log_stats()
```

## Common Debugging Scenarios

### 1. Session Not Starting

**Symptoms:**
- `start_session()` returns false
- Console shows "Failed to start session" error

**Debug Steps:**
```gdscript
# Enable detailed logging
logger.set_log_level(TorrentLogger.DEBUG)
logger.enable_category(TorrentLogger.SESSION, true)

# Check for port conflicts
session.set_listen_port_range(6881, 6891)

# Check console for specific error
var success = session.start_session()
if not success:
    print("Check console for error details")
```

**Common Causes:**
- Port already in use
- Insufficient permissions
- Missing dependencies (OpenSSL, Boost)

### 2. Torrent Not Adding

**Symptoms:**
- `add_torrent_file()` returns null handle
- No download progress

**Debug Steps:**
```gdscript
logger.set_log_level(TorrentLogger.DEBUG)
logger.enable_category(TorrentLogger.TORRENT, true)
logger.enable_category(TorrentLogger.STORAGE, true)

var handle = session.add_torrent_file(data, save_path)
if handle == null or not handle.is_valid():
    # Check console for error:
    # - "Session not running"
    # - "Save path cannot be empty"
    # - "Invalid torrent file"
    # - "libtorrent error: ..."
    pass
```

**Common Causes:**
- Session not started
- Invalid torrent file
- Invalid save path
- Disk space issues
- Permission denied

### 3. No Peers Connecting

**Symptoms:**
- `num_peers` stays at 0
- No download progress

**Debug Steps:**
```gdscript
logger.set_log_level(TorrentLogger.DEBUG)
logger.enable_category(TorrentLogger.PEER, true)
logger.enable_category(TorrentLogger.TRACKER, true)
logger.enable_category(TorrentLogger.DHT, true)

# Check DHT status
if not session.is_dht_running():
    session.start_dht()

# Check tracker status
var status = handle.get_status()
print("Announce: ", status.get_next_announce())

# Check peer info
var peers = handle.get_peer_info()
print("Peer count: ", peers.size())
```

**Common Causes:**
- DHT not started
- Firewall blocking connections
- No working trackers
- Port forwarding not configured

### 4. Slow Download Speed

**Symptoms:**
- Download rate below expected
- Connections dropping

**Debug Steps:**
```gdscript
logger.set_log_level(TorrentLogger.DEBUG)
logger.enable_category(TorrentLogger.PERFORMANCE, true)
logger.enable_category(TorrentLogger.PEER, true)

# Check current rates
var status = handle.get_status()
print("Download rate: ", status.get_download_rate() / 1024.0, " KB/s")
print("Upload rate: ", status.get_upload_rate() / 1024.0, " KB/s")
print("Peers: ", status.get_num_peers())
print("Seeds: ", status.get_num_seeds())

# Adjust limits
session.set_download_rate_limit(0)  # Unlimited
session.set_upload_rate_limit(0)    # Unlimited
session.set_max_connections(200)
```

**Common Causes:**
- Rate limits too low
- Connection limits too low
- Few seeds available
- Network congestion

### 5. Handle Becomes Invalid

**Symptoms:**
- `handle.is_valid()` returns false
- Operations fail with "Invalid handle" error

**Debug Steps:**
```gdscript
logger.set_log_level(TorrentLogger.DEBUG)
logger.enable_category(TorrentLogger.TORRENT, true)

# Always check handle validity
if not handle.is_valid():
    print("Handle is invalid - was torrent removed?")
    return

# Store handle reference
var stored_handles = []
stored_handles.append(handle)  # Prevent GC
```

**Common Causes:**
- Torrent was removed
- Session was stopped
- Handle was garbage collected
- Handle reference not stored

## Performance Debugging

### Monitoring Session Performance

```gdscript
func _process(_delta):
    if not session.is_running():
        return

    var stats = session.get_session_stats()

    # Monitor statistics
    logger.log_debug("Session stats: " + str(stats), "PERFORMANCE")
```

### Monitoring Torrent Performance

```gdscript
func monitor_torrent(handle: TorrentHandle):
    if not handle.is_valid():
        return

    var status = handle.get_status()

    var info = "Torrent: %s\n" % handle.get_name()
    info += "Progress: %.2f%%\n" % (status.get_progress() * 100)
    info += "Download: %.2f KB/s\n" % (status.get_download_rate() / 1024.0)
    info += "Upload: %.2f KB/s\n" % (status.get_upload_rate() / 1024.0)
    info += "Peers: %d (%d seeds)\n" % [status.get_num_peers(), status.get_num_seeds()]
    info += "State: %d" % status.get_state()

    logger.log_debug(info, "PERFORMANCE")
```

## Best Practices

### 1. Development vs Production

**Development:**
```gdscript
logger.enable_logging(true)
logger.set_log_level(TorrentLogger.DEBUG)
logger.set_log_file("user://debug.log")
```

**Production:**
```gdscript
logger.enable_logging(true)
logger.set_log_level(TorrentLogger.WARNING)  # Errors and warnings only
# No file logging (performance)
```

### 2. Logging Performance

When logging is disabled (`NONE`), there is **zero** performance overhead.

Tips for minimal overhead:
- Use `WARNING` or `ERROR` level in production
- Disable unused categories
- Avoid file logging in tight loops
- Use appropriate log levels for messages

### 3. Log Message Format

All logs follow this format:
```
[YYYY-MM-DD HH:MM:SS] [LEVEL] [CATEGORY] Message
```

Example:
```
[2025-10-04 15:30:45] [INFO] [SESSION] Logger attached to TorrentSession
[2025-10-04 15:30:45] [ERROR] [TORRENT] Failed to add torrent: invalid file
```

### 4. Filtering Logs

Use categories to focus on specific areas:

```gdscript
# Debug tracker issues only
logger.disable_all_categories()
logger.enable_category(TorrentLogger.TRACKER, true)
logger.set_log_level(TorrentLogger.DEBUG)

# Debug peer connection issues
logger.disable_all_categories()
logger.enable_category(TorrentLogger.PEER, true)
logger.enable_category(TorrentLogger.DHT, true)
logger.set_log_level(TorrentLogger.DEBUG)
```

## Troubleshooting Checklist

Before reporting issues, check:

1. **Logging Enabled**
   - [ ] Logger created and attached
   - [ ] Logging enabled with appropriate level
   - [ ] Relevant categories enabled

2. **Session State**
   - [ ] `session.is_running()` returns true
   - [ ] No error messages in console
   - [ ] DHT started if needed

3. **Handle Validity**
   - [ ] `handle.is_valid()` returns true
   - [ ] Handle reference stored (not GC'd)
   - [ ] Torrent not removed

4. **Console Errors**
   - [ ] Check Godot console for error messages
   - [ ] All errors follow `[ClassName::method]` format
   - [ ] Check for libtorrent error codes

5. **Log File**
   - [ ] File logging enabled if needed
   - [ ] Log file accessible and readable
   - [ ] Contains relevant timestamps

## Example: Complete Debug Session

```gdscript
extends Node

var session: TorrentSession
var logger: TorrentLogger
var handle: TorrentHandle

func _ready():
    setup_logging()
    setup_session()
    load_torrent()

func setup_logging():
    logger = TorrentLogger.new()
    logger.enable_logging(true)
    logger.set_log_level(TorrentLogger.DEBUG)
    logger.set_log_file("user://torrent_debug.log")

    # Focus on torrent and tracker issues
    logger.disable_all_categories()
    logger.enable_category(TorrentLogger.SESSION, true)
    logger.enable_category(TorrentLogger.TORRENT, true)
    logger.enable_category(TorrentLogger.TRACKER, true)

    logger.log_info("=== Debug session started ===", "APP")

func setup_session():
    session = TorrentSession.new()
    session.set_logger(logger)

    if not session.start_session():
        logger.log_error("Failed to start session", "APP")
        return

    logger.log_info("Session started successfully", "APP")

    # Start DHT
    session.start_dht()

func load_torrent():
    var file = FileAccess.open("res://test.torrent", FileAccess.READ)
    if file == null:
        logger.log_error("Failed to open torrent file", "APP")
        return

    var torrent_data = file.get_buffer(file.get_length())
    file.close()

    var save_path = "downloads"
    handle = session.add_torrent_file(torrent_data, save_path)

    if handle == null or not handle.is_valid():
        logger.log_error("Failed to add torrent", "APP")
        return

    logger.log_info("Torrent added: " + handle.get_name(), "APP")

func _process(_delta):
    if handle != null and handle.is_valid():
        var status = handle.get_status()
        logger.log_debug("Progress: %.2f%%" % (status.get_progress() * 100), "APP")

func _exit_tree():
    if logger != null:
        var stats = logger.get_log_stats()
        logger.log_info("=== Debug session ended ===", "APP")
        logger.log_info("Total logs: " + str(stats["total_count"]), "APP")
        logger.close_log_file()

    if session != null:
        session.stop_session()
```

## Additional Resources

- **Error Handling Guide**: See `ERROR_HANDLING.md` for error codes and recovery
- **Godot Console**: Press F3 in editor to view console output
- **Log Files**: Check `user://` directory for log files

## Summary

The logging system provides:
- ✅ Five log levels (NONE to TRACE)
- ✅ Category filtering for focused debugging
- ✅ Console and file output
- ✅ Zero overhead when disabled
- ✅ Detailed statistics
- ✅ Standardized message format

Enable logging during development, use appropriate levels in production, and leverage categories to focus on specific issues.
