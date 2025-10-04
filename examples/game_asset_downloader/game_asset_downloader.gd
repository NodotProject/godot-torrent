extends Control

# Game Asset Downloader Example
# Demonstrates downloading game assets (DLC, mods, maps) with priority management

var session: TorrentSession
var asset_packs: Array = []
var current_download = null

@onready var asset_list: ItemList = $VBoxContainer/AssetSection/AssetList
@onready var download_selected_button: Button = $VBoxContainer/AssetSection/DownloadSelectedButton
@onready var progress_container: VBoxContainer = $VBoxContainer/ProgressContainer
@onready var pack_name_label: Label = $VBoxContainer/ProgressContainer/PackNameLabel
@onready var overall_progress: ProgressBar = $VBoxContainer/ProgressContainer/OverallProgress
@onready var file_progress_container: VBoxContainer = $VBoxContainer/ProgressContainer/FileProgressContainer
@onready var speed_label: Label = $VBoxContainer/ProgressContainer/SpeedLabel
@onready var status_label: Label = $VBoxContainer/StatusLabel
@onready var install_path_input: LineEdit = $VBoxContainer/PathSection/InstallPathInput

var file_progress_bars: Dictionary = {}

func _ready():
	# Initialize session
	session = TorrentSession.new()

	# Configure for game asset downloading
	var settings = {
		"enable_dht": true,
		"download_rate_limit": 10 * 1024 * 1024,  # 10 MB/s
		"connections_limit": 200,  # More connections for faster downloads
	}
	session.start_session_with_settings(settings)

	# Setup example asset packs
	_setup_asset_packs()

	# Set default install path
	install_path_input.text = OS.get_user_data_dir() + "/game_assets"

	# Connect signals
	download_selected_button.pressed.connect(_on_download_selected_pressed)

	# Hide progress initially
	progress_container.visible = false

	# Start update timer
	var timer = Timer.new()
	timer.wait_time = 0.5  # Update twice per second for smooth progress
	timer.timeout.connect(_update_download_progress)
	add_child(timer)
	timer.start()

	status_label.text = "Session started. Select asset packs to download."

func _setup_asset_packs():
	# Example asset packs (in real game, these would come from a server/catalog)
	asset_packs = [
		{
			"name": "High-Res Texture Pack",
			"description": "4K textures for all game assets",
			"size_mb": 2500,
			"magnet": "magnet:?xt=urn:btih:example1...",
			"priority_files": [0, 1],  # High priority for core textures
		},
		{
			"name": "Winter Map DLC",
			"description": "New winter-themed maps and assets",
			"size_mb": 800,
			"magnet": "magnet:?xt=urn:btih:example2...",
			"priority_files": [0],
		},
		{
			"name": "Character Customization Pack",
			"description": "Additional character models and skins",
			"size_mb": 450,
			"magnet": "magnet:?xt=urn:btih:example3...",
			"priority_files": [],
		},
		{
			"name": "Sound & Music Expansion",
			"description": "Extended soundtrack and sound effects",
			"size_mb": 1200,
			"magnet": "magnet:?xt=urn:btih:example4...",
			"priority_files": [0, 1, 2],
		},
		{
			"name": "Bonus Campaign",
			"description": "5 additional campaign missions",
			"size_mb": 650,
			"magnet": "magnet:?xt=urn:btih:example5...",
			"priority_files": [0],
		},
	]

	# Populate asset list
	for pack in asset_packs:
		var item_text = "%s (%.1f MB)\n%s" % [pack["name"], pack["size_mb"], pack["description"]]
		asset_list.add_item(item_text)

func _on_download_selected_pressed():
	var selected = asset_list.get_selected_items()
	if selected.is_empty():
		status_label.text = "Please select an asset pack to download"
		return

	var pack_index = selected[0]
	var pack = asset_packs[pack_index]

	# Create install directory
	var install_path = install_path_input.text.strip_edges()
	if not DirAccess.dir_exists_absolute(install_path):
		DirAccess.make_dir_recursive_absolute(install_path)

	# Start download
	var handle = session.add_magnet_uri(pack["magnet"], install_path)

	if not handle or not handle.is_valid():
		status_label.text = "Error: Failed to start download"
		return

	# Set file priorities
	# Priority 7 = highest, 4 = normal, 1 = low
	if pack.has("priority_files"):
		var info = handle.get_torrent_info()
		if info and info.is_valid():
			var file_count = info.get_file_count()

			# Set all files to normal priority first
			for i in range(file_count):
				handle.set_file_priority(i, 4)

			# Set high priority files
			for file_idx in pack["priority_files"]:
				if file_idx < file_count:
					handle.set_file_priority(file_idx, 7)

	current_download = {
		"handle": handle,
		"pack": pack,
	}

	# Show progress UI
	progress_container.visible = true
	pack_name_label.text = "Downloading: " + pack["name"]
	download_selected_button.disabled = true

	# Setup file progress bars
	_setup_file_progress_bars(handle)

	status_label.text = "Download started..."

func _setup_file_progress_bars(handle):
	# Clear existing progress bars
	for child in file_progress_container.get_children():
		child.queue_free()
	file_progress_bars.clear()

	# Wait a moment for torrent info
	await get_tree().create_timer(1.0).timeout

	var info = handle.get_torrent_info()
	if not info or not info.is_valid():
		return

	# Create progress bars for each file
	var file_count = info.get_file_count()
	for i in range(min(file_count, 10)):  # Limit to 10 files for UI space
		var file_name = info.get_file_name(i)
		var priority = handle.get_file_priority(i)

		var vbox = VBoxContainer.new()
		var label = Label.new()
		label.text = "%s %s" % [file_name, "[HIGH PRIORITY]" if priority >= 7 else ""]
		var pbar = ProgressBar.new()

		vbox.add_child(label)
		vbox.add_child(pbar)
		file_progress_container.add_child(vbox)

		file_progress_bars[i] = pbar

func _update_download_progress():
	if not current_download:
		return

	var handle = current_download["handle"]
	if not handle or not handle.is_valid():
		return

	var status = handle.get_status()
	if not status:
		return

	# Update overall progress
	var progress = status.get_progress()
	overall_progress.value = progress * 100.0

	# Update speed
	var download_rate = status.get_download_rate() / 1024.0 / 1024.0
	var upload_rate = status.get_upload_rate() / 1024.0 / 1024.0
	var num_peers = status.get_num_peers()

	speed_label.text = "Download: %.2f MB/s | Upload: %.2f MB/s | Peers: %d" % [
		download_rate, upload_rate, num_peers
	]

	# Update file progress (if available)
	# Note: Individual file progress requires file_progress() method
	# For now, we show estimated progress based on file priorities

	# Check if complete
	var state = status.get_state_string()
	if state == "finished" or state == "seeding":
		status_label.text = "Download complete! Asset pack installed."
		download_selected_button.disabled = false
		download_selected_button.text = "Download Complete - Select Another"

func _notification(what):
	if what == NOTIFICATION_WM_CLOSE_REQUEST:
		if session:
			session.stop_session()
