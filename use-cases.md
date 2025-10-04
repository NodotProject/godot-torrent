# Use Case Examples

Real-world examples of using Godot-Torrent in different scenarios.

## Table of Contents

1. [Game Asset Distribution](#1-game-asset-distribution)
2. [P2P Multiplayer Content](#2-p2p-multiplayer-content)
3. [DLC and Update System](#3-dlc-and-update-system)
4. [Community Content Sharing](#4-community-content-sharing)
5. [Backup and Sync System](#5-backup-and-sync-system)

---

## 1. Game Asset Distribution

**Scenario**: Distribute large game assets (textures, models, audio) without expensive CDN costs.

### Implementation

```gdscript
extends Node

class_name AssetDownloader

var session: TorrentSession
var pending_assets: Dictionary = {}  # asset_id -> handle

signal asset_downloaded(asset_id: String, path: String)
signal download_progress(asset_id: String, progress: float)

func _ready():
    session = TorrentSession.new()
    session.start_session()
    session.start_dht()

    # Configure for asset downloads
    session.set_download_rate_limit(0)  # Max speed
    session.set_max_connections(200)

func download_asset_pack(asset_id: String, magnet_uri: String):
    var save_path = "user://assets/" + asset_id

    # Create directory
    var dir = DirAccess.open("user://assets")
    if not dir.dir_exists(asset_id):
        dir.make_dir(asset_id)

    # Start download
    var handle = session.add_magnet_uri(magnet_uri, save_path)
    if handle and handle.is_valid():
        pending_assets[asset_id] = handle
        print("📦 Downloading asset pack: ", asset_id)

func _process(_delta):
    for asset_id in pending_assets.keys():
        var handle = pending_assets[asset_id]
        if not handle or not handle.is_valid():
            continue

        var status = handle.get_status()
        if not status:
            continue

        # Emit progress
        download_progress.emit(asset_id, status.get_progress())

        # Check if complete
        if status.is_finished():
            var info = handle.get_torrent_info()
            asset_downloaded.emit(asset_id, "user://assets/" + asset_id)
            pending_assets.erase(asset_id)
            print("✅ Asset pack downloaded: ", asset_id)

# Usage in game
func load_map_assets():
    var downloader = AssetDownloader.new()
    add_child(downloader)

    downloader.asset_downloaded.connect(_on_assets_ready)
    downloader.download_progress.connect(_on_download_progress)

    downloader.download_asset_pack(
        "map_tropical_island",
        "magnet:?xt=urn:btih:..."
    )

func _on_assets_ready(asset_id: String, path: String):
    # Load and use assets
    print("Loading assets from: ", path)
```

### Benefits

- **Cost Effective**: Users share bandwidth
- **Scalable**: More downloaders = more bandwidth
- **Resilient**: No single point of failure
- **Fast**: Parallel downloads from multiple peers

---

## 2. P2P Multiplayer Content

**Scenario**: Share user-generated content (maps, mods, skins) in multiplayer games.

### Implementation

```gdscript
extends Node

class_name ContentSharing

var session: TorrentSession

# Share content you created
func share_custom_map(map_path: String) -> String:
    # 1. Create .torrent file (external tool for now)
    # 2. Upload .torrent to your server
    # 3. Get magnet link
    # 4. Start seeding

    var torrent_data = _create_torrent(map_path)  # External tool needed
    var handle = session.add_torrent_file(torrent_data, map_path)

    if handle and handle.is_valid():
        var info_hash = handle.get_info_hash()
        print("🌱 Seeding map: ", info_hash)
        return _generate_magnet_link(info_hash)

    return ""

# Download content shared by others
func download_community_map(magnet: String, map_id: String):
    var save_path = "user://community_maps/" + map_id
    var handle = session.add_magnet_uri(magnet, save_path)

    if handle and handle.is_valid():
        print("📥 Downloading community map: ", map_id)

# In-game lobby system
class_name GameLobby

var content_sharing: ContentSharing

func host_game_with_custom_map(map_file: String):
    # Share map with other players
    var magnet = content_sharing.share_custom_map(map_file)

    # Include magnet in game lobby data
    var lobby_data = {
        "map_magnet": magnet,
        "map_name": "My Custom Map"
    }

    _create_multiplayer_lobby(lobby_data)

func join_game_with_custom_map(lobby_data: Dictionary):
    # Download host's custom map
    var magnet = lobby_data["map_magnet"]
    content_sharing.download_community_map(magnet, "temp_map")

    # Wait for download, then join game
    await _wait_for_download_complete()
    _join_multiplayer_game()
```

---

## 3. DLC and Update System

**Scenario**: Deliver DLC and game updates efficiently using BitTorrent.

### Implementation

```gdscript
extends Node

class_name UpdateManager

var session: TorrentSession
var current_version: String = "1.0.0"

func check_for_updates():
    # Fetch update manifest from your server
    var manifest = await _fetch_update_manifest()

    if manifest.version > current_version:
        print("🔄 Update available: v%s" % manifest.version)
        _prompt_user_for_update(manifest)

func download_update(manifest: Dictionary):
    print("⬇️ Downloading update v%s..." % manifest.version)

    var handle = session.add_magnet_uri(
        manifest.magnet_uri,
        "user://updates/" + manifest.version
    )

    if not handle:
        push_error("Failed to start update download")
        return

    # Monitor progress
    while true:
        await get_tree().create_timer(1.0).timeout

        var status = handle.get_status()
        if not status:
            continue

        if status.is_finished():
            _apply_update(manifest.version)
            break

        var progress = status.get_progress() * 100.0
        print("Update progress: %.1f%%" % progress)

func _apply_update(version: String):
    print("✅ Update downloaded, applying...")

    # Extract files
    var update_path = "user://updates/" + version
    _extract_update_files(update_path)

    # Mark as applied
    current_version = version
    _save_version_info()

    print("🎉 Update applied! Please restart.")

# Update manifest example (JSON from your server)
# {
#   "version": "1.1.0",
#   "magnet_uri": "magnet:?xt=...",
#   "size": 150000000,
#   "changelog": "- New levels\\n- Bug fixes"
# }
```

---

## 4. Community Content Sharing

**Scenario**: Build a workshop/marketplace for user-created content.

### Implementation

```gdscript
extends Control

class_name WorkshopBrowser

var session: TorrentSession
var content_db: Array = []  # Array of {id, name, magnet, author, downloads}

@onready var content_list: ItemList = $ContentList
@onready var download_btn: Button = $DownloadBtn

func _ready():
    session = TorrentSession.new()
    session.start_session()
    session.start_dht()

    _load_workshop_content()

func _load_workshop_content():
    # Fetch content list from your API
    var response = await _fetch_api("https://api.yourgame.com/workshop")

    content_db = response.items
    _populate_list()

func _populate_list():
    content_list.clear()

    for item in content_db:
        content_list.add_item("%s by %s (%d downloads)" % [
            item.name,
            item.author,
            item.downloads
        ])

func _on_download_clicked():
    var selected = content_list.get_selected_items()[0]
    var item = content_db[selected]

    _download_workshop_item(item)

func _download_workshop_item(item: Dictionary):
    var save_path = "user://workshop/" + item.id

    var handle = session.add_magnet_uri(item.magnet, save_path)

    if handle:
        _track_download(handle, item)
        print("📥 Downloading: ", item.name)

func _track_download(handle: TorrentHandle, item: Dictionary):
    # Create download UI
    var progress_bar = ProgressBar.new()
    add_child(progress_bar)

    # Update progress
    while true:
        await get_tree().create_timer(0.5).timeout

        var status = handle.get_status()
        if not status:
            break

        progress_bar.value = status.get_progress() * 100.0

        if status.is_finished():
            _on_workshop_item_ready(item)
            progress_bar.queue_free()
            break

func _on_workshop_item_ready(item: Dictionary):
    print("✅ Workshop item ready: ", item.name)

    # Notify API (increment download count)
    _increment_download_count(item.id)

    # Install/activate the content
    _install_workshop_content(item)
```

---

## 5. Backup and Sync System

**Scenario**: P2P backup system for game saves across devices.

### Implementation

```gdscript
extends Node

class_name SaveSync

var session: TorrentSession
var save_dir: String = "user://saves"

func backup_saves():
    # Create torrent from save directory
    var torrent_data = _create_save_backup_torrent()

    if torrent_data.is_empty():
        push_error("Failed to create backup")
        return

    # Start seeding
    var handle = session.add_torrent_file(torrent_data, save_dir)

    if handle and handle.is_valid():
        var info_hash = handle.get_info_hash()

        # Save magnet link for restore
        _save_backup_reference(info_hash)

        print("💾 Backup created: ", info_hash)

func restore_saves_from_backup(magnet: String):
    print("📥 Restoring saves from backup...")

    var temp_dir = "user://temp_restore"
    var handle = session.add_magnet_uri(magnet, temp_dir)

    if not handle:
        push_error("Failed to start restore")
        return

    # Wait for download
    while true:
        await get_tree().create_timer(1.0).timeout

        var status = handle.get_status()
        if not status:
            continue

        if status.is_finished():
            _copy_restored_saves(temp_dir, save_dir)
            print("✅ Saves restored!")
            break

func sync_across_devices():
    # On device A: Create backup and get magnet
    var magnet = backup_saves()

    # Store magnet in cloud (your server/database)
    await _upload_magnet_to_cloud(magnet)

    # On device B: Fetch magnet and restore
    var synced_magnet = await _fetch_magnet_from_cloud()
    restore_saves_from_backup(synced_magnet)

func _create_save_backup_torrent() -> PackedByteArray:
    # Use external torrent creation tool
    # Returns .torrent file data
    return PackedByteArray()  # Placeholder

func _copy_restored_saves(from: String, to: String):
    var dir = DirAccess.open(from)
    # Copy files from temp to saves directory
```

---

## Common Patterns

### Pattern 1: Download with Fallback

```gdscript
func download_with_fallback(magnet: String, http_url: String, save_path: String):
    # Try BitTorrent first
    var handle = session.add_magnet_uri(magnet, save_path)

    # Set timeout
    var timeout = 60.0  # 60 seconds
    var elapsed = 0.0

    while elapsed < timeout:
        await get_tree().create_timer(1.0).timeout
        elapsed += 1.0

        var status = handle.get_status()
        if status and status.get_num_peers() > 0:
            # Peers found, continue with torrent
            return await _wait_for_torrent_complete(handle)

    # Fallback to HTTP
    print("⚠️ No peers found, falling back to HTTP")
    session.remove_torrent(handle, true)
    return await _download_via_http(http_url, save_path)
```

### Pattern 2: Progressive Loading

```gdscript
func download_for_streaming(handle: TorrentHandle):
    # Download first and last pieces for seeking
    var info = handle.get_torrent_info()
    var piece_count = info.get_piece_count()

    handle.set_piece_priority(0, 7)  # First piece
    handle.set_piece_priority(piece_count - 1, 7)  # Last piece

    # Download next pieces sequentially
    for i in range(1, min(10, piece_count)):
        handle.set_piece_priority(i, 7)

    # Start playback as soon as first pieces are ready
    await _wait_for_first_pieces()
    _start_streaming_playback()
```

### Pattern 3: Bandwidth Scheduling

```gdscript
func apply_bandwidth_schedule():
    var hour = Time.get_time_dict_from_system()["hour"]

    if hour >= 9 and hour < 17:  # Business hours
        session.set_download_rate_limit(1024 * 1024)  # 1 MB/s
    else:  # Off hours
        session.set_download_rate_limit(0)  # Unlimited
```

---

## Best Practices

1. **Always provide fallback options** (HTTP, direct download)
2. **Cache magnets/torrents** for offline capability
3. **Implement retry logic** for failed downloads
4. **Monitor bandwidth usage** to avoid overwhelming connections
5. **Clean up completed downloads** to free resources
6. **Use DHT** for better peer discovery
7. **Seed what you download** to support the network
8. **Version your content** for easy updates

---

**More Examples**: See the `examples/` directory in the repository for complete working projects.

---

**Related**:
- [Basic Download Tutorial](tutorial-basic-download.md)
- [Advanced Configuration](guide-advanced-config.md)
- [API Reference](api-reference.md)
