# Error Handling Guide

## Overview

The godot-torrent library implements a standardized error handling system that provides:
- **Consistent error reporting** across all classes and methods
- **Error codes and categories** for programmatic error handling
- **Human-readable error messages** with context
- **Mapping from libtorrent errors** to GDScript format
- **Recovery information** to help users handle errors appropriately

## Error System Components

### 1. TorrentError Class

The `TorrentError` class represents a single error with detailed information.

**Key Properties:**
- `code`: Integer error code (see error codes below)
- `category`: Error category (session, torrent, network, storage, etc.)
- `message`: Human-readable error description
- `context`: Additional context about where/when the error occurred
- `is_error()`: Returns true if this represents an actual error (code != OK)
- `is_recoverable()`: Returns true if the error can be recovered from

**Creating Errors:**
```gdscript
# Create from error code
var error = TorrentError.create(TorrentError.INVALID_TORRENT_FILE, "Custom message", "context")

# Create from libtorrent error
var error = TorrentError.from_libtorrent_error(error_code, error_message, "context")

# Convert to dictionary
var error_dict = error.to_dict()
print(error_dict)  # { "code": 201, "category": "torrent", "message": "...", ... }
```

### 2. TorrentResult Class

The `TorrentResult` class is a wrapper for operations that can fail, similar to Rust's `Result<T, E>` type.

**Key Methods:**
- `is_ok()`: Returns true if the operation succeeded
- `is_error()`: Returns true if the operation failed
- `get_value()`: Returns the success value (or null if error)
- `get_error()`: Returns the TorrentError (or null if success)
- `unwrap()`: Returns value or prints error to console
- `unwrap_or(default)`: Returns value or default if error

**Usage Pattern:**
```gdscript
# Methods that can fail return TorrentResult
var result = some_operation_that_can_fail()

if result.is_ok():
    var value = result.get_value()
    print("Success: ", value)
else:
    var error = result.get_error()
    print("Error: ", error.get_message())
    print("Category: ", error.get_category())
    print("Recoverable: ", error.is_recoverable())
```

## Error Categories

Errors are organized into categories for easier handling:

| Category | Code Range | Description |
|----------|------------|-------------|
| `NONE` | 0 | No error (success) |
| `SESSION_ERROR` | 100-199 | Session lifecycle and configuration errors |
| `TORRENT_ERROR` | 200-299 | Torrent-specific errors |
| `NETWORK_ERROR` | 300-399 | Network connectivity and timeout errors |
| `STORAGE_ERROR` | 400-499 | File system and storage errors |
| `PARSE_ERROR` | 500-599 | Data parsing and format errors |
| `VALIDATION_ERROR` | 600-699 | Input validation errors |
| `TRACKER_ERROR` | 700-799 | Tracker communication errors |
| `DHT_ERROR` | 800-899 | DHT-related errors |
| `PEER_ERROR` | 900-999 | Peer connection errors |
| `INTERNAL_ERROR` | 1000+ | Internal library errors |

## Common Error Codes

### Session Errors (100-199)
- `SESSION_NOT_RUNNING` (100): Attempted operation when session is not running
- `SESSION_ALREADY_RUNNING` (101): Attempted to start an already running session
- `SESSION_START_FAILED` (102): Failed to initialize session
- `SESSION_STOP_FAILED` (103): Failed to stop session cleanly

### Torrent Errors (200-299)
- `INVALID_TORRENT_HANDLE` (200): Torrent handle is invalid or expired
- `INVALID_TORRENT_FILE` (201): Torrent file is malformed or corrupt
- `INVALID_MAGNET_URI` (202): Magnet URI is malformed
- `TORRENT_ADD_FAILED` (203): Failed to add torrent to session
- `TORRENT_REMOVE_FAILED` (204): Failed to remove torrent from session
- `TORRENT_NOT_FOUND` (205): Torrent not found in session

### Storage Errors (400-499)
- `INVALID_PATH` (400): Path is malformed or invalid
- `PATH_NOT_FOUND` (401): Specified path does not exist
- `PERMISSION_DENIED` (402): Insufficient permissions for operation
- `DISK_FULL` (403): Not enough disk space
- `STORAGE_MOVE_FAILED` (404): Failed to move torrent storage
- `FILE_RENAME_FAILED` (405): Failed to rename file

### Validation Errors (600-699)
- `INVALID_PARAMETER` (600): Invalid parameter value
- `EMPTY_SAVE_PATH` (601): Save path is empty or null
- `INVALID_PIECE_INDEX` (602): Piece index out of range
- `INVALID_FILE_INDEX` (603): File index out of range
- `INVALID_PRIORITY` (604): Priority value out of valid range
- `INVALID_URL` (605): URL is malformed

## Error Handling Best Practices

### 1. Always Check for Errors

```gdscript
var session = TorrentSession.new()
var success = session.start_session()
if not success:
    # Handle error - check Godot console for detailed message
    print("Failed to start session")
    return
```

### 2. Check Return Values

Operations that can fail will log errors to the Godot console via `push_error()`:

```gdscript
var handle = session.add_torrent_file(torrent_data, save_path)
if handle == null or not handle.is_valid():
    # Error occurred - check console for details
    print("Failed to add torrent")
    return
```

### 3. Validate Inputs

Always validate inputs before calling library methods:

```gdscript
# Bad - will cause error
var handle = session.add_torrent_file(data, "")  # Empty path

# Good - validate first
if save_path.is_empty():
    print("Error: Save path cannot be empty")
    return

var handle = session.add_torrent_file(data, save_path)
```

### 4. Check Session State

```gdscript
if not session.is_running():
    print("Session not running - start it first")
    session.start_session()
```

### 5. Validate Handle Before Use

```gdscript
if not handle.is_valid():
    print("Handle is invalid - cannot perform operation")
    return

handle.pause()  # Safe to use
```

## Error Messages

All errors are reported to the Godot console using `push_error()` with a standardized format:

```
[ClassName::method_name] Error message
```

Examples:
```
[TorrentSession::add_torrent_file] Session not running
[TorrentSession::add_torrent_file] Save path cannot be empty
[TorrentSession::add_torrent_file] libtorrent error 123: invalid torrent file
[TorrentHandle::pause] Invalid handle
[TorrentHandle::set_piece_priority] Exception: out of range
```

## Libtorrent Error Mapping

Libtorrent errors are automatically mapped to godot-torrent error codes:

| Libtorrent Error | Mapped Code | Category |
|------------------|-------------|----------|
| Invalid torrent file | `INVALID_TORRENT_FILE` | `TORRENT_ERROR` |
| Invalid magnet | `INVALID_MAGNET_URI` | `TORRENT_ERROR` |
| Permission denied | `PERMISSION_DENIED` | `STORAGE_ERROR` |
| Disk full | `DISK_FULL` | `STORAGE_ERROR` |
| Timeout | `TIMEOUT` | `NETWORK_ERROR` |
| Tracker error | `TRACKER_ANNOUNCE_FAILED` | `TRACKER_ERROR` |
| Connection failed | `CONNECTION_FAILED` | `NETWORK_ERROR` |

## Recovery Strategies

### Recoverable Errors

These errors can typically be retried or worked around:
- Network errors (timeout, connection failed)
- Tracker errors
- Some DHT errors

**Strategy:** Retry with backoff, try alternative trackers, or continue without that resource.

### Non-Recoverable Errors

These errors require user intervention:
- Validation errors (invalid input)
- Parse errors (corrupt data)
- Storage errors (disk full, permission denied)
- Internal errors

**Strategy:** Report error to user, log details, and request corrective action.

## Example: Complete Error Handling

```gdscript
extends Node

var session: TorrentSession
var handle: TorrentHandle

func _ready():
    session = TorrentSession.new()

    # Start session with error checking
    if not session.start_session():
        push_error("Failed to start torrent session")
        return

    # Load torrent file
    var file = FileAccess.open("res://test.torrent", FileAccess.READ)
    if file == null:
        push_error("Failed to open torrent file")
        session.stop_session()
        return

    var torrent_data = file.get_buffer(file.get_length())
    file.close()

    # Add torrent with validation
    var save_path = "downloads"
    if save_path.is_empty():
        push_error("Save path is empty")
        session.stop_session()
        return

    handle = session.add_torrent_file(torrent_data, save_path)
    if handle == null or not handle.is_valid():
        push_error("Failed to add torrent - check console for details")
        session.stop_session()
        return

    print("Torrent added successfully: ", handle.get_name())

func _exit_tree():
    if session != null:
        session.stop_session()
```

## Future Enhancements

Planned improvements to the error handling system:

1. **Return TorrentResult from more methods** - Gradually migrate from boolean/null returns to TorrentResult for better error information
2. **Error handling signals** - Emit signals for errors to allow centralized error handling
3. **Error recovery helpers** - Built-in retry logic and recovery strategies
4. **Structured error data** - Additional structured data in errors for programmatic handling
5. **Error statistics** - Track error frequency and patterns for debugging

## Summary

The standardized error handling system provides:
- ✅ Consistent error codes and categories
- ✅ Human-readable error messages with context
- ✅ Mapping from libtorrent errors to GDScript format
- ✅ Error recovery information
- ✅ Console logging via `push_error()`
- ✅ Standardized error message format

Always check return values, validate inputs, and monitor the Godot console for detailed error information.
