extends Control

# Torrent Browser Example
# Demonstrates browsing torrent contents and selective file downloading

var session: TorrentSession
var current_handle = null

@onready var magnet_input: LineEdit = $VBoxContainer/InputSection/MagnetInput
@onready var load_button: Button = $VBoxContainer/InputSection/LoadButton
@onready var torrent_name_label: Label = $VBoxContainer/InfoSection/TorrentNameLabel
@onready var torrent_size_label: Label = $VBoxContainer/InfoSection/TorrentSizeLabel
@onready var file_tree: Tree = $VBoxContainer/FileTree
@onready var download_button: Button = $VBoxContainer/DownloadSection/DownloadButton
@onready var download_path_input: LineEdit = $VBoxContainer/DownloadSection/DownloadPathInput
@onready var status_label: Label = $VBoxContainer/StatusLabel
@onready var progress_bar: ProgressBar = $VBoxContainer/ProgressSection/ProgressBar
@onready var progress_label: Label = $VBoxContainer/ProgressSection/ProgressLabel

var file_priorities: Array = []

func _ready():
	# Initialize session
	session = TorrentSession.new()
	session.start_session()

	# Set default download path
	download_path_input.text = OS.get_user_data_dir() + "/downloads"

	# Connect signals
	load_button.pressed.connect(_on_load_button_pressed)
	download_button.pressed.connect(_on_download_button_pressed)
	file_tree.item_edited.connect(_on_file_tree_item_edited)

	# Setup file tree
	file_tree.set_columns(3)
	file_tree.set_column_title(0, "File Name")
	file_tree.set_column_title(1, "Size")
	file_tree.set_column_title(2, "Download")
	file_tree.set_column_titles_visible(true)

	# Hide download controls initially
	download_button.disabled = true
	progress_bar.visible = false
	progress_label.visible = false

	# Start update timer
	var timer = Timer.new()
	timer.wait_time = 1.0
	timer.timeout.connect(_update_progress)
	add_child(timer)
	timer.start()

	status_label.text = "Session started. Enter a magnet URI to browse."

func _on_load_button_pressed():
	var magnet_uri = magnet_input.text.strip_edges()

	if magnet_uri.is_empty() or not magnet_uri.begins_with("magnet:"):
		status_label.text = "Error: Invalid magnet URI"
		return

	# Add torrent in metadata-only mode (pause immediately)
	var temp_path = OS.get_cache_dir() + "/godot_torrent_temp"
	current_handle = session.add_magnet_uri(magnet_uri, temp_path)

	if not current_handle or not current_handle.is_valid():
		status_label.text = "Error: Failed to load torrent"
		return

	# Pause immediately to only fetch metadata
	current_handle.pause()

	status_label.text = "Loading torrent metadata..."
	load_button.disabled = true

	# Wait for metadata
	await get_tree().create_timer(2.0).timeout
	_display_torrent_info()

func _display_torrent_info():
	if not current_handle or not current_handle.is_valid():
		status_label.text = "Error: Invalid torrent handle"
		return

	var info = current_handle.get_torrent_info()
	if not info or not info.is_valid():
		status_label.text = "Error: Metadata not yet available. Please wait..."
		return

	# Display basic info
	var name = info.get_name()
	var total_size = info.get_total_size()
	var file_count = info.get_file_count()

	torrent_name_label.text = "Name: " + name
	torrent_size_label.text = "Size: %.2f MB (%d files)" % [total_size / 1024.0 / 1024.0, file_count]

	# Populate file tree
	file_tree.clear()
	var root = file_tree.create_item()
	file_tree.hide_root = true

	file_priorities.clear()
	file_priorities.resize(file_count)

	for i in range(file_count):
		var file_name = info.get_file_name(i)
		var file_size = info.get_file_size(i)

		var item = file_tree.create_item(root)
		item.set_text(0, file_name)
		item.set_text(1, "%.2f MB" % (file_size / 1024.0 / 1024.0))
		item.set_cell_mode(2, TreeItem.CELL_MODE_CHECK)
		item.set_checked(2, true)
		item.set_editable(2, true)
		item.set_metadata(0, i)  # Store file index

		file_priorities[i] = 4  # Default priority

	download_button.disabled = false
	status_label.text = "Torrent loaded. Select files to download."
	load_button.disabled = false

func _on_file_tree_item_edited():
	# Update file priorities based on checkbox state
	var edited_item = file_tree.get_edited()
	var file_index = edited_item.get_metadata(0)
	var is_checked = edited_item.is_checked(2)

	file_priorities[file_index] = 4 if is_checked else 0  # 4 = normal, 0 = skip

func _on_download_button_pressed():
	if not current_handle or not current_handle.is_valid():
		status_label.text = "Error: No torrent loaded"
		return

	var download_path = download_path_input.text.strip_edges()

	# Create download directory
	if not DirAccess.dir_exists_absolute(download_path):
		DirAccess.make_dir_recursive_absolute(download_path)

	# Apply file priorities
	for i in range(file_priorities.size()):
		current_handle.set_file_priority(i, file_priorities[i])

	# Start download
	current_handle.resume()

	# Show progress
	progress_bar.visible = true
	progress_label.visible = true
	download_button.disabled = true

	status_label.text = "Download started..."

func _update_progress():
	if not current_handle or not current_handle.is_valid():
		return

	var status = current_handle.get_status()
	if not status:
		return

	var progress = status.get_progress()
	var download_rate = status.get_download_rate() / 1024.0 / 1024.0
	var num_peers = status.get_num_peers()
	var state = status.get_state_string()

	progress_bar.value = progress * 100.0
	progress_label.text = "Progress: %.1f%% - %.2f MB/s - %d peers - %s" % [
		progress * 100.0, download_rate, num_peers, state
	]

	if state == "finished" or state == "seeding":
		status_label.text = "Download complete!"
		download_button.text = "Download Complete"

func _notification(what):
	if what == NOTIFICATION_WM_CLOSE_REQUEST:
		if session:
			session.stop_session()
