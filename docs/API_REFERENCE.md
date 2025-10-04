# API Reference

Complete API documentation for godot-torrent library.

## Table of Contents

1. [Quick Start](#quick-start)
2. [Core Classes](#core-classes)
   - [TorrentSession](#torrentsession)
   - [TorrentHandle](#torrenthandle)
   - [TorrentInfo](#torrentinfo)
   - [TorrentStatus](#torrentstatus)
3. [Error Handling](#error-handling)
   - [TorrentError](#torrenterror)
   - [TorrentResult](#torrentresult)
4. [Logging](#logging)
   - [TorrentLogger](#torrentlogger)
5. [Peer Management](#peer-management)
   - [PeerInfo](#peerinfo)
6. [Code Examples](#code-examples)

---

## Quick Start

```gdscript
extends Node

var session: TorrentSession
var handle: TorrentHandle

func _ready():
    # Create and start session
    session = TorrentSession.new()
    if not session.start_session():
        push_error("Failed to start session")
        return

    # Start DHT for peer discovery
    session.start_dht()

    # Load torrent file
    var file = FileAccess.open("res://example.torrent", FileAccess.READ)
    if file == null:
        push_error("Failed to open torrent file")
        return

    var torrent_data = file.get_buffer(file.get_length())
    file.close()

    # Add torrent
    handle = session.add_torrent_file(torrent_data, "downloads")
    if handle == null or not handle.is_valid():
        push_error("Failed to add torrent")
        return

    print("Torrent added: ", handle.get_name())

func _process(_delta):
    if handle != null and handle.is_valid():
        var status = handle.get_status()
        print("Progress: %.2f%%" % (status.get_progress() * 100))

func _exit_tree():
    if session != null:
        session.stop_session()
```

---

## Core Classes

## TorrentSession

The main session class that manages all torrent operations.

### Constructor

```gdscript
var session = TorrentSession.new()
```

### Session Lifecycle

#### `bool start_session()`
Starts the torrent session with default settings.

**Returns:** `true` if successful, `false` on error

**Example:**
```gdscript
var session = TorrentSession.new()
if not session.start_session():
    push_error("Failed to start session")
```

**Errors:**
- Session already running
- Port binding failure
- System resource error

---

#### `bool start_session_with_settings(Dictionary settings)`
Starts the session with custom settings.

**Parameters:**
- `settings` (Dictionary): Custom session settings

**Returns:** `true` if successful, `false` on error

**Example:**
```gdscript
var settings = {
    "listen_interfaces": "0.0.0.0:6881",
    "enable_dht": true,
    "download_rate_limit": 1048576  # 1 MB/s
}
if not session.start_session_with_settings(settings):
    push_error("Failed to start session")
```

---

#### `void stop_session()`
Stops the session and cleans up resources.

**Example:**
```gdscript
session.stop_session()
```

**Note:** This will remove all torrents and close all connections. Use `save_state()` before stopping to preserve session data.

---

#### `bool is_running()`
Checks if the session is currently running.

**Returns:** `true` if running, `false` otherwise

**Example:**
```gdscript
if session.is_running():
    print("Session is active")
```

---

### Configuration

#### `void set_download_rate_limit(int bytes_per_second)`
Sets the global download rate limit.

**Parameters:**
- `bytes_per_second` (int): Download limit in bytes/second (0 = unlimited)

**Example:**
```gdscript
session.set_download_rate_limit(1048576)  # 1 MB/s
session.set_download_rate_limit(0)        # Unlimited
```

---

#### `void set_upload_rate_limit(int bytes_per_second)`
Sets the global upload rate limit.

**Parameters:**
- `bytes_per_second` (int): Upload limit in bytes/second (0 = unlimited)

**Example:**
```gdscript
session.set_upload_rate_limit(524288)  # 512 KB/s
```

---

#### `void set_listen_port(int port)`
Sets the listening port for incoming connections.

**Parameters:**
- `port` (int): Port number (1-65535)

**Example:**
```gdscript
session.set_listen_port(6881)
```

---

#### `void set_listen_port_range(int min_port, int max_port)`
Sets a range of ports to try for listening.

**Parameters:**
- `min_port` (int): Minimum port number
- `max_port` (int): Maximum port number

**Example:**
```gdscript
session.set_listen_port_range(6881, 6891)
```

---

#### `void set_max_connections(int limit)`
Sets the maximum number of connections.

**Parameters:**
- `limit` (int): Maximum connections (recommended: 200)

**Example:**
```gdscript
session.set_max_connections(200)
```

---

#### `void set_max_uploads(int limit)`
Sets the maximum number of upload slots.

**Parameters:**
- `limit` (int): Maximum upload slots (recommended: 4-8)

**Example:**
```gdscript
session.set_max_uploads(4)
```

---

### DHT Management

#### `bool is_dht_running()`
Checks if DHT is currently running.

**Returns:** `true` if DHT is active, `false` otherwise

---

#### `void start_dht()`
Starts the DHT for peer discovery.

**Example:**
```gdscript
session.start_dht()
```

**Note:** DHT is essential for magnet links and trackerless torrents.

---

#### `void stop_dht()`
Stops the DHT.

**Example:**
```gdscript
session.stop_dht()
```

---

#### `Dictionary get_dht_state()`
Gets current DHT state information.

**Returns:** Dictionary with keys:
- `running` (bool): Whether DHT is active
- `nodes` (int): Number of DHT nodes

**Example:**
```gdscript
var state = session.get_dht_state()
print("DHT nodes: ", state["nodes"])
```

---

#### `void add_dht_node(String host, int port)`
Adds a DHT bootstrap node.

**Parameters:**
- `host` (String): Node hostname or IP
- `port` (int): Node port

**Example:**
```gdscript
session.add_dht_node("router.bittorrent.com", 6881)
```

---

#### `PackedByteArray save_dht_state()`
Saves DHT state for later restoration.

**Returns:** PackedByteArray containing DHT state

**Example:**
```gdscript
var dht_data = session.save_dht_state()
# Save to file for next session
```

---

#### `bool load_dht_state(PackedByteArray dht_data)`
Loads previously saved DHT state.

**Parameters:**
- `dht_data` (PackedByteArray): Saved DHT state

**Returns:** `true` if successful, `false` on error

**Example:**
```gdscript
var dht_data = load_dht_from_file()
session.load_dht_state(dht_data)
```

---

### Torrent Operations

#### `TorrentHandle add_torrent_file(PackedByteArray torrent_data, String save_path)`
Adds a torrent from .torrent file data.

**Parameters:**
- `torrent_data` (PackedByteArray): Torrent file contents
- `save_path` (String): Download directory path

**Returns:** `TorrentHandle` on success, `null` on error

**Example:**
```gdscript
var file = FileAccess.open("res://file.torrent", FileAccess.READ)
var torrent_data = file.get_buffer(file.get_length())
file.close()

var handle = session.add_torrent_file(torrent_data, "downloads")
if handle != null and handle.is_valid():
    print("Added: ", handle.get_name())
```

**Errors:**
- Session not running
- Save path empty or invalid
- Invalid torrent file
- Torrent already exists

---

#### `TorrentHandle add_torrent_file_with_resume(PackedByteArray torrent_data, String save_path, PackedByteArray resume_data)`
Adds a torrent with resume data for faster startup.

**Parameters:**
- `torrent_data` (PackedByteArray): Torrent file contents
- `save_path` (String): Download directory path
- `resume_data` (PackedByteArray): Previously saved resume data

**Returns:** `TorrentHandle` on success, `null` on error

**Example:**
```gdscript
var torrent_data = load_torrent_file()
var resume_data = load_resume_data()
var handle = session.add_torrent_file_with_resume(torrent_data, "downloads", resume_data)
```

---

#### `TorrentHandle add_magnet_uri(String magnet_uri, String save_path)`
Adds a torrent from a magnet link.

**Parameters:**
- `magnet_uri` (String): Magnet link (magnet:?xt=...)
- `save_path` (String): Download directory path

**Returns:** `TorrentHandle` on success, `null` on error

**Example:**
```gdscript
var magnet = "magnet:?xt=urn:btih:..."
var handle = session.add_magnet_uri(magnet, "downloads")
```

**Note:** DHT must be running for magnet links to work.

---

#### `TorrentHandle add_magnet_uri_with_resume(String magnet_uri, String save_path, PackedByteArray resume_data)`
Adds a magnet link with resume data.

**Parameters:**
- `magnet_uri` (String): Magnet link
- `save_path` (String): Download directory path
- `resume_data` (PackedByteArray): Previously saved resume data

**Returns:** `TorrentHandle` on success, `null` on error

---

#### `bool remove_torrent(TorrentHandle handle, bool delete_files = false)`
Removes a torrent from the session.

**Parameters:**
- `handle` (TorrentHandle): Torrent to remove
- `delete_files` (bool): Whether to delete downloaded files

**Returns:** `true` if successful, `false` on error

**Example:**
```gdscript
# Remove torrent but keep files
session.remove_torrent(handle, false)

# Remove torrent and delete all files
session.remove_torrent(handle, true)
```

---

### Alerts

#### `Array get_alerts()`
Gets pending alerts from the session.

**Returns:** Array of alert dictionaries

**Example:**
```gdscript
var alerts = session.get_alerts()
for alert in alerts:
    print("Alert: ", alert["message"])
```

**Alert Properties:**
- `message` (String): Alert message
- `type` (int): Alert type ID
- `what` (String): Alert category

---

#### `void clear_alerts()`
Clears all pending alerts.

**Example:**
```gdscript
session.clear_alerts()
```

---

#### `void post_torrent_updates()`
Requests status updates for all torrents.

**Example:**
```gdscript
session.post_torrent_updates()
# Check alerts for state_update_alert
```

---

### State Persistence

#### `PackedByteArray save_state()`
Saves complete session state.

**Returns:** PackedByteArray containing session state

**Example:**
```gdscript
var state = session.save_state()
var file = FileAccess.open("user://session.dat", FileAccess.WRITE)
file.store_buffer(state)
file.close()
```

---

#### `bool load_state(PackedByteArray state_data)`
Loads previously saved session state.

**Parameters:**
- `state_data` (PackedByteArray): Saved session state

**Returns:** `true` if successful, `false` on error

**Example:**
```gdscript
var file = FileAccess.open("user://session.dat", FileAccess.READ)
var state = file.get_buffer(file.get_length())
file.close()

session.load_state(state)
```

---

### Logging

#### `void set_logger(TorrentLogger logger)`
Attaches a logger to the session.

**Parameters:**
- `logger` (TorrentLogger): Logger instance

**Example:**
```gdscript
var logger = TorrentLogger.new()
logger.enable_logging(true)
session.set_logger(logger)
```

---

#### `TorrentLogger get_logger()`
Gets the current logger.

**Returns:** `TorrentLogger` or `null` if none attached

---

#### `void enable_logging(bool enabled)`
Enables or disables logging.

**Parameters:**
- `enabled` (bool): Whether to enable logging

---

#### `void set_log_level(int level)`
Sets the log level.

**Parameters:**
- `level` (int): Log level (use TorrentLogger constants)

**Example:**
```gdscript
session.set_log_level(TorrentLogger.DEBUG)
```

---

## TorrentHandle

Represents an individual torrent.

### Validity

#### `bool is_valid()`
Checks if the handle is still valid.

**Returns:** `true` if valid, `false` otherwise

**Example:**
```gdscript
if handle.is_valid():
    print("Handle is valid")
```

**Note:** Always check validity before using a handle.

---

### Basic Control

#### `void pause()`
Pauses the torrent.

**Example:**
```gdscript
handle.pause()
```

---

#### `void resume()`
Resumes the torrent.

**Example:**
```gdscript
handle.resume()
```

---

#### `bool is_paused()`
Checks if the torrent is paused.

**Returns:** `true` if paused, `false` otherwise

---

### Information

#### `String get_name()`
Gets the torrent name.

**Returns:** Torrent name string

**Example:**
```gdscript
print("Name: ", handle.get_name())
```

---

#### `String get_info_hash()`
Gets the torrent info hash (SHA-1).

**Returns:** 40-character hexadecimal hash

**Example:**
```gdscript
print("Info hash: ", handle.get_info_hash())
```

---

#### `TorrentInfo get_torrent_info()`
Gets detailed torrent information.

**Returns:** `TorrentInfo` object

**Example:**
```gdscript
var info = handle.get_torrent_info()
print("Total size: ", info.get_total_size())
```

---

#### `TorrentStatus get_status()`
Gets current torrent status.

**Returns:** `TorrentStatus` object

**Example:**
```gdscript
var status = handle.get_status()
print("Progress: %.2f%%" % (status.get_progress() * 100))
```

---

### File Management

#### `void set_file_priority(int file_index, int priority)`
Sets download priority for a specific file.

**Parameters:**
- `file_index` (int): File index (0-based)
- `priority` (int): Priority (0-7, 0=don't download, 7=highest)

**Example:**
```gdscript
# Don't download file 0
handle.set_file_priority(0, 0)

# High priority for file 1
handle.set_file_priority(1, 7)
```

---

#### `int get_file_priority(int file_index)`
Gets the priority of a specific file.

**Parameters:**
- `file_index` (int): File index

**Returns:** Priority value (0-7)

---

#### `void rename_file(int file_index, String new_name)`
Renames a file in the torrent.

**Parameters:**
- `file_index` (int): File index
- `new_name` (String): New filename

**Example:**
```gdscript
handle.rename_file(0, "renamed.mp4")
```

---

#### `Array get_file_progress()`
Gets download progress for each file.

**Returns:** Array of integers (bytes downloaded per file)

**Example:**
```gdscript
var progress = handle.get_file_progress()
for i in range(progress.size()):
    print("File %d: %d bytes" % [i, progress[i]])
```

---

### Piece Management

#### `void set_piece_priority(int piece_index, int priority)`
Sets priority for a specific piece.

**Parameters:**
- `piece_index` (int): Piece index
- `priority` (int): Priority (0-7)

---

#### `int get_piece_priority(int piece_index)`
Gets priority of a specific piece.

**Parameters:**
- `piece_index` (int): Piece index

**Returns:** Priority value (0-7)

---

#### `bool have_piece(int piece_index)`
Checks if a piece has been downloaded.

**Parameters:**
- `piece_index` (int): Piece index

**Returns:** `true` if piece is complete, `false` otherwise

---

#### `void read_piece(int piece_index)`
Requests to read a piece from disk.

**Parameters:**
- `piece_index` (int): Piece index

**Note:** Check alerts for `read_piece_alert` with the data.

---

#### `Array get_piece_availability()`
Gets piece availability across all peers.

**Returns:** Array of integers (peer count per piece)

---

### Peer Management

#### `Array get_peer_info()`
Gets information about connected peers.

**Returns:** Array of `PeerInfo` objects

**Example:**
```gdscript
var peers = handle.get_peer_info()
print("Connected peers: ", peers.size())
for peer in peers:
    print("Peer IP: ", peer.get_ip())
```

---

### Tracker Management

#### `void add_tracker(String url, int tier = 0)`
Adds a tracker to the torrent.

**Parameters:**
- `url` (String): Tracker URL
- `tier` (int): Tracker tier (0=primary)

**Example:**
```gdscript
handle.add_tracker("http://tracker.example.com:80/announce", 0)
```

---

#### `void remove_tracker(String url)`
Removes a tracker from the torrent.

**Parameters:**
- `url` (String): Tracker URL to remove

---

#### `Array get_trackers()`
Gets all trackers for the torrent.

**Returns:** Array of tracker dictionaries

**Example:**
```gdscript
var trackers = handle.get_trackers()
for tracker in trackers:
    print("Tracker: ", tracker["url"])
```

---

### Advanced Operations

#### `void force_recheck()`
Forces a recheck of all downloaded data.

**Example:**
```gdscript
handle.force_recheck()
```

---

#### `void force_reannounce()`
Forces immediate tracker announce.

**Example:**
```gdscript
handle.force_reannounce()
```

---

#### `void force_dht_announce()`
Forces DHT announce.

**Example:**
```gdscript
handle.force_dht_announce()
```

---

#### `void move_storage(String new_path)`
Moves torrent storage to new location.

**Parameters:**
- `new_path` (String): New storage path

**Example:**
```gdscript
handle.move_storage("/new/downloads/path")
```

---

#### `void scrape_tracker()`
Requests scrape from tracker.

---

#### `void flush_cache()`
Flushes cached data to disk.

---

#### `void clear_error()`
Clears torrent error state.

---

### Resume Data

#### `void save_resume_data()`
Requests resume data to be generated.

**Note:** Check alerts for `save_resume_data_alert` containing the data.

**Example:**
```gdscript
handle.save_resume_data()
# Later, check alerts for resume data
```

---

## TorrentInfo

Provides detailed information about a torrent.

### Basic Information

#### `String get_name()`
Gets the torrent name.

**Returns:** Torrent name

---

#### `String get_comment()`
Gets the torrent comment.

**Returns:** Comment string

---

#### `String get_creator()`
Gets the torrent creator.

**Returns:** Creator string

---

#### `int64 get_creation_date()`
Gets the creation timestamp.

**Returns:** Unix timestamp

---

### Size Information

#### `int64 get_total_size()`
Gets total size in bytes.

**Returns:** Total size

**Example:**
```gdscript
var info = handle.get_torrent_info()
var size_mb = info.get_total_size() / 1048576.0
print("Size: %.2f MB" % size_mb)
```

---

#### `int64 get_piece_length()`
Gets the piece size in bytes.

**Returns:** Piece size

---

#### `int get_num_pieces()`
Gets the total number of pieces.

**Returns:** Piece count

---

### File Information

#### `int get_num_files()`
Gets the number of files in the torrent.

**Returns:** File count

---

#### `Array get_files()`
Gets information about all files.

**Returns:** Array of file dictionaries with keys:
- `path` (String): File path
- `size` (int64): File size in bytes

**Example:**
```gdscript
var info = handle.get_torrent_info()
var files = info.get_files()
for file in files:
    print("File: %s (%d bytes)" % [file["path"], file["size"]])
```

---

## TorrentStatus

Provides real-time status information about a torrent.

### Progress

#### `float get_progress()`
Gets download progress (0.0 to 1.0).

**Returns:** Progress fraction

**Example:**
```gdscript
var status = handle.get_status()
print("Progress: %.2f%%" % (status.get_progress() * 100))
```

---

### State

#### `int get_state()`
Gets the current torrent state.

**Returns:** State code:
- `0`: Checking files
- `1`: Downloading metadata
- `2`: Downloading
- `3`: Finished
- `4`: Seeding
- `5`: Allocating
- `6`: Checking resume data

---

#### `bool is_paused()`
Checks if torrent is paused.

**Returns:** `true` if paused

---

#### `bool is_finished()`
Checks if download is complete.

**Returns:** `true` if finished

---

#### `bool is_seeding()`
Checks if torrent is seeding.

**Returns:** `true` if seeding

---

### Rates

#### `int get_download_rate()`
Gets current download rate.

**Returns:** Bytes per second

**Example:**
```gdscript
var rate_kbps = status.get_download_rate() / 1024.0
print("Download: %.2f KB/s" % rate_kbps)
```

---

#### `int get_upload_rate()`
Gets current upload rate.

**Returns:** Bytes per second

---

### Totals

#### `int64 get_total_download()`
Gets total bytes downloaded.

**Returns:** Total downloaded bytes

---

#### `int64 get_total_upload()`
Gets total bytes uploaded.

**Returns:** Total uploaded bytes

---

#### `int64 get_total_wanted()`
Gets total bytes to download (excluding unwanted files).

**Returns:** Total wanted bytes

---

#### `int64 get_total_done()`
Gets bytes downloaded so far.

**Returns:** Downloaded bytes

---

### Peers

#### `int get_num_peers()`
Gets number of connected peers.

**Returns:** Peer count

---

#### `int get_num_seeds()`
Gets number of connected seeds.

**Returns:** Seed count

---

### Time

#### `int get_next_announce()`
Gets seconds until next tracker announce.

**Returns:** Seconds

---

---

## Error Handling

## TorrentError

Represents an error with code, category, and message.

See `ERROR_HANDLING.md` for complete documentation.

### Creating Errors

```gdscript
var error = TorrentError.create(TorrentError.INVALID_TORRENT_FILE, "Custom message")
```

### Properties

- `get_code()`: Error code
- `get_category()`: Error category
- `get_message()`: Error message
- `is_error()`: Is this an error?
- `is_recoverable()`: Can it be recovered from?

---

## TorrentResult

Wrapper for operations that can fail.

See `ERROR_HANDLING.md` for complete documentation.

### Usage

```gdscript
var result = some_operation()
if result.is_ok():
    var value = result.get_value()
else:
    var error = result.get_error()
    print("Error: ", error.get_message())
```

---

## Logging

## TorrentLogger

Centralized logging for debugging.

See `DEBUGGING_GUIDE.md` for complete documentation.

### Quick Start

```gdscript
var logger = TorrentLogger.new()
logger.enable_logging(true)
logger.set_log_level(TorrentLogger.DEBUG)
logger.set_log_file("user://debug.log")

session.set_logger(logger)
```

### Log Levels

- `NONE`: Disabled
- `ERROR`: Errors only
- `WARNING`: Warnings + errors
- `INFO`: Info + above
- `DEBUG`: Debug + above
- `TRACE`: Verbose + above

---

## Peer Management

## PeerInfo

Information about a connected peer.

### Methods

#### `String get_ip()`
Gets peer IP address.

**Returns:** IP string

---

#### `int get_port()`
Gets peer port.

**Returns:** Port number

---

#### `String get_client()`
Gets peer client name.

**Returns:** Client string

---

#### `float get_progress()`
Gets peer's download progress.

**Returns:** Progress (0.0-1.0)

---

#### `int get_download_rate()`
Gets download rate from this peer.

**Returns:** Bytes per second

---

#### `int get_upload_rate()`
Gets upload rate to this peer.

**Returns:** Bytes per second

---

## Code Examples

### Complete Download Manager

```gdscript
extends Node

var session: TorrentSession
var logger: TorrentLogger
var active_torrents: Array[TorrentHandle] = []

func _ready():
    setup_logging()
    setup_session()

func setup_logging():
    logger = TorrentLogger.new()
    logger.enable_logging(true)
    logger.set_log_level(TorrentLogger.INFO)

func setup_session():
    session = TorrentSession.new()
    session.set_logger(logger)

    if not session.start_session():
        push_error("Failed to start session")
        return

    # Configure session
    session.set_download_rate_limit(0)  # Unlimited
    session.set_max_connections(200)
    session.start_dht()

func add_torrent_file(path: String, save_path: String) -> TorrentHandle:
    var file = FileAccess.open(path, FileAccess.READ)
    if file == null:
        push_error("Failed to open: " + path)
        return null

    var data = file.get_buffer(file.get_length())
    file.close()

    var handle = session.add_torrent_file(data, save_path)
    if handle != null and handle.is_valid():
        active_torrents.append(handle)

    return handle

func add_magnet(uri: String, save_path: String) -> TorrentHandle:
    var handle = session.add_magnet_uri(uri, save_path)
    if handle != null and handle.is_valid():
        active_torrents.append(handle)

    return handle

func _process(_delta):
    update_torrents()

func update_torrents():
    for handle in active_torrents:
        if not handle.is_valid():
            continue

        var status = handle.get_status()
        var progress = status.get_progress() * 100
        var dl_rate = status.get_download_rate() / 1024.0

        print("%s: %.1f%% - %.1f KB/s" % [
            handle.get_name(), progress, dl_rate
        ])

func _exit_tree():
    if session != null:
        session.stop_session()
```

---

### Selective File Download

```gdscript
func download_specific_files(handle: TorrentHandle, file_indices: Array):
    if not handle.is_valid():
        return

    var info = handle.get_torrent_info()
    var num_files = info.get_num_files()

    # Disable all files
    for i in range(num_files):
        handle.set_file_priority(i, 0)

    # Enable only selected files
    for index in file_indices:
        if index >= 0 and index < num_files:
            handle.set_file_priority(index, 7)  # Highest priority
```

---

### Resume Data Management

```gdscript
func save_all_resume_data():
    for handle in active_torrents:
        if handle.is_valid():
            handle.save_resume_data()

    # Check alerts for resume data
    await get_tree().create_timer(1.0).timeout

    var alerts = session.get_alerts()
    for alert in alerts:
        if alert.has("resume_data"):
            save_resume_data_to_file(alert["info_hash"], alert["resume_data"])

func load_resume_data(info_hash: String) -> PackedByteArray:
    var path = "user://resume/" + info_hash + ".dat"
    var file = FileAccess.open(path, FileAccess.READ)
    if file == null:
        return PackedByteArray()

    var data = file.get_buffer(file.get_length())
    file.close()
    return data
```

---

## Additional Resources

- **Error Handling**: See `ERROR_HANDLING.md`
- **Debugging**: See `DEBUGGING_GUIDE.md`
- **Examples**: See `examples/` directory (if available)
- **libtorrent Docs**: https://libtorrent.org/reference.html

---

## Summary

This API provides:
- ✅ Complete session management
- ✅ Torrent operations (files and magnets)
- ✅ DHT support
- ✅ File and piece priority control
- ✅ Peer management
- ✅ Resume data for fast restarts
- ✅ Comprehensive error handling
- ✅ Integrated logging

All methods include error checking via console output. Always check return values and handle validity before operations.
