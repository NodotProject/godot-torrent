# Tutorial: Building a Basic Download Manager

In this tutorial, you'll build a complete torrent download manager with a user interface. This is a practical, real-world example that demonstrates core Godot-Torrent functionality.

## What You'll Build

A download manager with:
- ✅ Add torrents via magnet links or .torrent files
- ✅ Display download progress with progress bars
- ✅ Show download/upload speeds and peer counts
- ✅ Pause and resume downloads
- ✅ Remove completed or unwanted torrents
- ✅ Automatic session management

## Prerequisites

- Completed the [Getting Started Guide](GETTING_STARTED.md)
- Basic knowledge of Godot UI nodes
- Understanding of signals in Godot

---

## Part 1: Project Setup

### Step 1: Create the Project Structure

```
your_project/
├── scenes/
│   ├── main.tscn          # Main scene
│   └── torrent_item.tscn  # Individual torrent UI
├── scripts/
│   ├── download_manager.gd
│   └── torrent_item.gd
└── addons/
    └── godot-torrent/     # The GDExtension
```

### Step 2: Create the Main Scene

Create a new scene (`scenes/main.tscn`) with this structure:

```
Control (MarginContainer)
├── VBoxContainer
│   ├── HBoxContainer (Top bar)
│   │   ├── LineEdit (for magnet links)
│   │   ├── Button ("Add Magnet")
│   │   └── Button ("Add File")
│   ├── ScrollContainer
│   │   └── VBoxContainer (TorrentList)
│   └── HBoxContainer (Status bar)
│       └── Label (StatusLabel)
```

---

## Part 2: The Download Manager Script

Create `scripts/download_manager.gd`:

```gdscript
extends Control

# UI References
@onready var magnet_input: LineEdit = $VBoxContainer/TopBar/MagnetInput
@onready var add_magnet_btn: Button = $VBoxContainer/TopBar/AddMagnetBtn
@onready var add_file_btn: Button = $VBoxContainer/TopBar/AddFileBtn
@onready var torrent_list: VBoxContainer = $VBoxContainer/ScrollContainer/TorrentList
@onready var status_label: Label = $VBoxContainer/StatusBar/StatusLabel

# Torrent session
var session: TorrentSession
var download_path: String = "user://downloads"

# Track active torrents
var active_torrents: Dictionary = {}  # handle -> torrent_item_node

# Preload the torrent item scene
var TorrentItem = preload("res://scenes/torrent_item.tscn")

func _ready():
    # Create download directory
    _ensure_download_directory()

    # Initialize torrent session
    _init_session()

    # Connect UI signals
    add_magnet_btn.pressed.connect(_on_add_magnet_pressed)
    add_file_btn.pressed.connect(_on_add_file_pressed)
    magnet_input.text_submitted.connect(_on_magnet_submitted)

    _update_status("Ready to download")

func _init_session():
    session = TorrentSession.new()

    if not session.start_session():
        _show_error("Failed to start torrent session")
        return

    # Enable DHT for better peer discovery
    session.start_dht()

    # Configure reasonable defaults
    session.set_max_connections(200)
    session.set_max_uploads(4)
    session.set_listen_port_range(6881, 6889)

    print("✅ Torrent session initialized")

func _ensure_download_directory():
    var dir = DirAccess.open("user://")
    if not dir.dir_exists("downloads"):
        dir.make_dir("downloads")

func _on_add_magnet_pressed():
    var magnet = magnet_input.text.strip_edges()
    if magnet.is_empty():
        _show_error("Please enter a magnet link")
        return

    _add_magnet(magnet)
    magnet_input.clear()

func _on_magnet_submitted(text: String):
    _on_add_magnet_pressed()

func _on_add_file_pressed():
    # Create file dialog
    var dialog = FileDialog.new()
    dialog.access = FileDialog.ACCESS_FILESYSTEM
    dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
    dialog.filters = PackedStringArray(["*.torrent ; Torrent Files"])
    dialog.file_selected.connect(_on_torrent_file_selected)
    add_child(dialog)
    dialog.popup_centered(Vector2(800, 600))

func _on_torrent_file_selected(path: String):
    var file = FileAccess.open(path, FileAccess.READ)
    if not file:
        _show_error("Failed to read torrent file")
        return

    var torrent_data = file.get_buffer(file.get_length())
    file.close()

    _add_torrent_file(torrent_data)

func _add_magnet(magnet: String):
    if not session or not session.is_running():
        _show_error("Session not running")
        return

    var handle = session.add_magnet_uri(magnet, download_path)

    if not handle or not handle.is_valid():
        _show_error("Failed to add magnet link")
        return

    _create_torrent_item(handle)
    _update_status("Torrent added - waiting for metadata...")

func _add_torrent_file(torrent_data: PackedByteArray):
    if not session or not session.is_running():
        _show_error("Session not running")
        return

    var handle = session.add_torrent_file(torrent_data, download_path)

    if not handle or not handle.is_valid():
        _show_error("Failed to add torrent file")
        return

    _create_torrent_item(handle)
    _update_status("Torrent added successfully")

func _create_torrent_item(handle: TorrentHandle):
    var item = TorrentItem.instantiate()
    torrent_list.add_child(item)
    item.initialize(handle, self)

    active_torrents[handle] = item

    print("📦 Torrent added, handle valid: ", handle.is_valid())

func remove_torrent(handle: TorrentHandle):
    if handle in active_torrents:
        var item = active_torrents[handle]
        item.queue_free()
        active_torrents.erase(handle)

    if session:
        session.remove_torrent(handle, false)  # Don't delete files

    _update_status("Torrent removed")

func _update_status(message: String):
    status_label.text = message

func _show_error(message: String):
    status_label.text = "❌ " + message
    push_error(message)

func _process(_delta):
    # Update active torrent count
    if active_torrents.size() > 0:
        _update_status("Active torrents: %d" % active_torrents.size())

func _exit_tree():
    # Clean up session
    if session:
        session.stop_session()
    print("👋 Download manager closed")
```

---

## Part 3: The Torrent Item UI

Create `scenes/torrent_item.tscn` with this structure:

```
PanelContainer
└── MarginContainer
    └── VBoxContainer
        ├── HBoxContainer (Header)
        │   ├── Label (NameLabel)
        │   └── Button (RemoveBtn "✕")
        ├── ProgressBar (ProgressBar)
        └── HBoxContainer (Stats)
            ├── Label (StatsLabel)
            └── HBoxContainer (Controls)
                ├── Button (PauseBtn)
                └── Button (ResumeBtn)
```

---

## Part 4: The Torrent Item Script

Create `scripts/torrent_item.gd`:

```gdscript
extends PanelContainer

# UI References
@onready var name_label: Label = $Margin/VBox/Header/NameLabel
@onready var remove_btn: Button = $Margin/VBox/Header/RemoveBtn
@onready var progress_bar: ProgressBar = $Margin/VBox/ProgressBar
@onready var stats_label: Label = $Margin/VBox/Stats/StatsLabel
@onready var pause_btn: Button = $Margin/VBox/Stats/Controls/PauseBtn
@onready var resume_btn: Button = $Margin/VBox/Stats/Controls/ResumeBtn

# Torrent data
var handle: TorrentHandle
var manager  # Reference to DownloadManager

# Update timer
var update_timer: float = 0.0
var update_interval: float = 1.0  # Update every second

func _ready():
    # Connect button signals
    remove_btn.pressed.connect(_on_remove_pressed)
    pause_btn.pressed.connect(_on_pause_pressed)
    resume_btn.pressed.connect(_on_resume_pressed)

    # Configure progress bar
    progress_bar.min_value = 0.0
    progress_bar.max_value = 100.0
    progress_bar.value = 0.0

    # Initially show pause button
    resume_btn.visible = false

func initialize(torrent_handle: TorrentHandle, download_manager):
    handle = torrent_handle
    manager = download_manager

    # Set initial name (will update when metadata arrives)
    name_label.text = "Downloading metadata..."

func _process(delta):
    if not handle or not handle.is_valid():
        return

    update_timer += delta
    if update_timer >= update_interval:
        update_timer = 0.0
        _update_display()

func _update_display():
    var status = handle.get_status()
    if not status:
        return

    # Update name if we have metadata
    var info = handle.get_torrent_info()
    if info and info.is_valid():
        var torrent_name = info.get_name()
        if torrent_name and not torrent_name.is_empty():
            name_label.text = torrent_name

    # Update progress
    var progress = status.get_progress() * 100.0
    progress_bar.value = progress

    # Update statistics
    var download_rate = status.get_download_rate() / 1024.0  # KB/s
    var upload_rate = status.get_upload_rate() / 1024.0      # KB/s
    var num_peers = status.get_num_peers()
    var state = status.get_state_string()

    stats_label.text = "%.1f%% | ⬇ %.1f KB/s | ⬆ %.1f KB/s | 👥 %d | %s" % [
        progress, download_rate, upload_rate, num_peers, state
    ]

    # Update button visibility based on pause state
    var is_paused = status.is_paused()
    pause_btn.visible = not is_paused
    resume_btn.visible = is_paused

    # Check if download is complete
    if status.is_finished():
        progress_bar.modulate = Color.GREEN
        stats_label.text = "✅ Download complete! | Seeding | ⬆ %.1f KB/s | 👥 %d" % [
            upload_rate, num_peers
        ]

func _on_pause_pressed():
    if handle and handle.is_valid():
        handle.pause()

func _on_resume_pressed():
    if handle and handle.is_valid():
        handle.resume()

func _on_remove_pressed():
    if manager:
        manager.remove_torrent(handle)
```

---

## Part 5: Styling the UI (Optional)

Add a custom theme to make it look better. Here's a simple theme:

```gdscript
# In your _ready() function of download_manager.gd

func _ready():
    _setup_theme()
    # ... rest of setup

func _setup_theme():
    # Create a simple dark theme
    var theme = Theme.new()

    # Panel style
    var panel_style = StyleBoxFlat.new()
    panel_style.bg_color = Color(0.2, 0.2, 0.2)
    panel_style.border_width_all = 1
    panel_style.border_color = Color(0.4, 0.4, 0.4)
    panel_style.corner_radius_all = 4
    theme.set_stylebox("panel", "PanelContainer", panel_style)

    # Apply theme
    self.theme = theme
```

---

## Part 6: Testing Your Download Manager

1. **Run the scene**
2. **Add a test magnet link** (public domain content):
   ```
   magnet:?xt=urn:btih:08ada5a7a6183aae1e09d831df6748d566095a10&dn=Sintel
   ```
3. **Watch the progress** in real-time
4. **Test pause/resume** functionality
5. **Remove the torrent** when done

---

## Part 7: Enhancements

### Add Bandwidth Controls

```gdscript
# In download_manager.gd

@onready var download_limit: SpinBox = $VBoxContainer/TopBar/DownloadLimit
@onready var upload_limit: SpinBox = $VBoxContainer/TopBar/UploadLimit

func _ready():
    # ... existing code ...

    download_limit.value_changed.connect(_on_download_limit_changed)
    upload_limit.value_changed.connect(_on_upload_limit_changed)

    # Set defaults (0 = unlimited)
    download_limit.value = 0
    upload_limit.value = 512  # 512 KB/s default

func _on_download_limit_changed(value: float):
    if session:
        var bytes_per_sec = int(value * 1024)  # Convert KB/s to bytes/s
        session.set_download_rate_limit(bytes_per_sec)

func _on_upload_limit_changed(value: float):
    if session:
        var bytes_per_sec = int(value * 1024)
        session.set_upload_rate_limit(bytes_per_sec)
```

### Add Torrent Queue

```gdscript
# Limit simultaneous downloads
var max_active_downloads: int = 3
var queued_torrents: Array[TorrentHandle] = []

func _add_magnet(magnet: String):
    var handle = session.add_magnet_uri(magnet, download_path)

    if not handle or not handle.is_valid():
        return

    if active_torrents.size() >= max_active_downloads:
        handle.pause()
        queued_torrents.append(handle)
        _update_status("Torrent queued")

    _create_torrent_item(handle)

func remove_torrent(handle: TorrentHandle):
    # ... existing code ...

    # Start next queued torrent
    if queued_torrents.size() > 0:
        var next_handle = queued_torrents.pop_front()
        if next_handle and next_handle.is_valid():
            next_handle.resume()
```

### Save/Load Session State

```gdscript
# Save on exit
func _exit_tree():
    _save_session_state()
    if session:
        session.stop_session()

func _save_session_state():
    if not session:
        return

    var state = session.save_state()
    var file = FileAccess.open("user://session.dat", FileAccess.WRITE)
    if file:
        file.store_buffer(state)
        file.close()
        print("💾 Session state saved")

# Load on start
func _init_session():
    session = TorrentSession.new()

    if not session.start_session():
        return

    _load_session_state()
    session.start_dht()

func _load_session_state():
    var file = FileAccess.open("user://session.dat", FileAccess.READ)
    if not file:
        return

    var state = file.get_buffer(file.get_length())
    file.close()

    if state.size() > 0:
        session.load_state(state)
        print("📂 Session state loaded")
```

---

## Complete Example

The complete working example is available in the repository:
- `examples/download_manager/` - Full source code
- `examples/download_manager/README.md` - Additional notes

---

## Troubleshooting

### Issue: Magnet links not connecting

**Solution**: Make sure DHT is enabled:
```gdscript
session.start_dht()
```

### Issue: No progress after adding torrent

**Possible causes**:
1. Waiting for metadata (normal for magnet links)
2. No peers available (check peer count)
3. Firewall blocking connections (check ports)

### Issue: Downloads are slow

**Solutions**:
1. Increase connection limits:
   ```gdscript
   session.set_max_connections(200)
   ```
2. Remove bandwidth limits:
   ```gdscript
   session.set_download_rate_limit(0)  # Unlimited
   ```
3. Wait for more peers to connect

---

## Next Steps

- **[Seeding Tutorial](TUTORIAL_SEEDING.md)** - Learn how to share files
- **[Advanced Configuration](GUIDE_ADVANCED_CONFIG.md)** - Fine-tune your setup
- **[Performance Tuning](GUIDE_PERFORMANCE_TUNING.md)** - Optimize for speed

---

**🎉 Congratulations!** You've built a complete torrent download manager!
