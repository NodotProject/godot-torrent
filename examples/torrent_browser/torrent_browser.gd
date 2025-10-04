extends Control

# Torrent Browser Example
# Demonstrates browsing torrent contents and selective file downloading

var session: TorrentSession
var current_handle = null
var is_loading_metadata: bool = false
var is_downloading: bool = false

@onready var magnet_input: LineEdit = $VBoxContainer/InputSection/MagnetInput
@onready var load_button: Button = $VBoxContainer/InputSection/LoadButton
@onready var torrent_name_label: Label = $VBoxContainer/InfoSection/TorrentNameLabel
@onready var torrent_size_label: Label = $VBoxContainer/InfoSection/TorrentSizeLabel
@onready var file_tree: Tree = $VBoxContainer/FileTree
@onready var select_all_button: Button = $VBoxContainer/FileSelectionButtons/SelectAllButton
@onready var select_none_button: Button = $VBoxContainer/FileSelectionButtons/SelectNoneButton
@onready var download_button: Button = $VBoxContainer/DownloadSection/DownloadButton
@onready var download_path_input: LineEdit = $VBoxContainer/DownloadSection/DownloadPathInput
@onready var status_label: Label = $VBoxContainer/StatusLabel
@onready var progress_bar: ProgressBar = $VBoxContainer/ProgressSection/ProgressBar
@onready var progress_label: Label = $VBoxContainer/ProgressSection/ProgressLabel

var file_priorities: Array = []

func _ready():
	# Initialize session (but don't start it yet)
	session = TorrentSession.new()
	print("Session created.")

	# Set default download path
	download_path_input.text = OS.get_user_data_dir() + "/downloads"

	# Connect signals
	load_button.pressed.connect(_on_load_button_pressed)
	download_button.pressed.connect(_on_download_button_pressed)
	file_tree.item_edited.connect(_on_file_tree_item_edited)
	select_all_button.pressed.connect(_on_select_all_pressed)
	select_none_button.pressed.connect(_on_select_none_pressed)
	# Note: metadata_received signal exists but we poll for metadata in _process() instead

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

	status_label.text = "Ready. Enter a magnet URI to browse."

func _process(delta):
	# Process alerts for metadata loading
	if is_loading_metadata and current_handle and current_handle.is_valid():
		session.post_torrent_updates()
		var alerts = session.get_alerts()
		
		for alert in alerts:
			if alert.has("torrent_status"):
				var statuses = alert["torrent_status"]
				for status_dict in statuses:
					if status_dict.get("info_hash") == current_handle.get_info_hash():
						var has_metadata = status_dict.get("has_metadata", false)
						var state = status_dict.get("state", 0)
						var num_peers = status_dict.get("num_peers", 0)
						
						if Engine.get_frames_drawn() % 60 == 0:
							print("State: %d (downloading_metadata), Peers: %d, Has metadata: %s" % [state, num_peers, has_metadata])
						
						if has_metadata:
							print("✓ Metadata received!")
							_display_torrent_info()
							current_handle.pause()
							load_button.disabled = false
							status_label.text = "Metadata loaded! Select files to download."
							is_loading_metadata = false
							return
	
	# Process alerts for downloading
	if is_downloading and current_handle and current_handle.is_valid():
		session.post_torrent_updates()
		var alerts = session.get_alerts()
		
		for alert in alerts:
			if alert.has("torrent_status"):
				var statuses = alert["torrent_status"]
				for status_dict in statuses:
					if status_dict.get("info_hash") == current_handle.get_info_hash():
						var progress = status_dict.get("progress", 0.0)
						var download_rate = status_dict.get("download_rate", 0)
						var num_peers = status_dict.get("num_peers", 0)
						var is_finished = status_dict.get("is_finished", false)
						
						progress_bar.value = progress * 100.0
						progress_label.text = "Progress: %.1f%% - %.2f MB/s - %d peers" % [
							progress * 100.0,
							download_rate / 1024.0 / 1024.0,
							num_peers
						]
						
						if is_finished:
							status_label.text = "Download complete!"
							download_button.text = "Download Complete"
							is_downloading = false
							return

func _on_load_button_pressed():
	print("Load button pressed.")
	var magnet_uri = magnet_input.text.strip_edges()

	if magnet_uri.is_empty() or not magnet_uri.begins_with("magnet:"):
		status_label.text = "Error: Invalid magnet URI"
		return

	print("Step 1: Checking session...")
	# Start session if not already started
	if not session.is_running():
		print("Step 2: Starting session...")
		if not session.start_session():
			status_label.text = "Error: Failed to start session"
			return
		print("Step 3: Session started.")
		
		print("Step 4: Starting DHT...")
		# Enable DHT for magnet link support
		session.start_dht()
		print("Step 5: DHT enabled.")

	print("Step 6: Adding magnet URI...")
	# Add torrent in metadata-only mode (pause immediately)
	var temp_path = OS.get_cache_dir() + "/godot_torrent_temp"
	print("  Save path: ", temp_path)
	current_handle = session.add_magnet_uri(magnet_uri, temp_path)
	print("Step 7: Magnet URI added.")

	if not current_handle or not current_handle.is_valid():
		print("Error: Failed to load torrent handle from session.")
		status_label.text = "Error: Failed to load torrent"
		return

	print("Step 8: Handle created successfully.")
	print("Step 9: Checking paused state...")
	print("Is paused: ", current_handle.is_paused())
	print("Step 10: Setting up UI...")
	
	# The torrent will start in "downloading metadata" state.
	# We will pause it once metadata is received.

	status_label.text = "Loading torrent metadata..."
	load_button.disabled = true
	is_loading_metadata = true
	
	print("Step 11: Done! Waiting for metadata...")
	
	# No longer waiting for metadata here, it will be handled by _process()

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
		var file_name = info.get_file_path_at(i)
		var file_size = info.get_file_size_at(i)

		var item = file_tree.create_item(root)
		item.set_text(0, file_name)
		item.set_text(1, "%.2f MB" % (file_size / 1024.0 / 1024.0))
		item.set_cell_mode(2, TreeItem.CELL_MODE_CHECK)
		item.set_checked(2, true)
		item.set_editable(2, true)
		item.set_metadata(0, i)  # Store file index

		file_priorities[i] = 4  # Default priority (will download)
	
	# Important: Set all file priorities to 0 (don't download) initially
	# User must explicitly select files, then they'll be set to priority 4 when download starts
	print("Pausing torrent and setting files to not download by default...")
	for i in range(file_count):
		current_handle.set_file_priority(i, 0)  # 0 = don't download

	download_button.disabled = false
	status_label.text = "Torrent loaded. Select files to download."
	load_button.disabled = false

func _on_file_tree_item_edited():
	# Update file priorities based on checkbox state
	var edited_item = file_tree.get_edited()
	var file_index = edited_item.get_metadata(0)
	var is_checked = edited_item.is_checked(2)

	file_priorities[file_index] = 4 if is_checked else 0  # 4 = normal, 0 = skip

func _on_select_all_pressed():
	var root = file_tree.get_root()
	if not root:
		return
	
	var item = root.get_first_child()
	while item:
		item.set_checked(2, true)
		var file_index = item.get_metadata(0)
		file_priorities[file_index] = 4
		item = item.get_next()
	
	print("All files selected")

func _on_select_none_pressed():
	var root = file_tree.get_root()
	if not root:
		return
	
	var item = root.get_first_child()
	while item:
		item.set_checked(2, false)
		var file_index = item.get_metadata(0)
		file_priorities[file_index] = 0
		item = item.get_next()
	
	print("All files deselected")

func _on_download_button_pressed():
	if not current_handle or not current_handle.is_valid():
		status_label.text = "Error: No torrent loaded"
		return

	var download_path = download_path_input.text.strip_edges()

	# Create download directory
	if not DirAccess.dir_exists_absolute(download_path):
		DirAccess.make_dir_recursive_absolute(download_path)

	# Move the torrent to the actual download location
	print("Moving torrent to download path: ", download_path)
	current_handle.move_storage(download_path)

	# Apply file priorities
	print("Applying file priorities...")
	for i in range(file_priorities.size()):
		var priority = file_priorities[i]
		print("  File %d: priority %d" % [i, priority])
		current_handle.set_file_priority(i, priority)
	print("File priorities applied.")

	# Start download
	current_handle.resume()
	
	# Enable download progress tracking
	is_downloading = true

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
	var has_metadata = status.has_metadata()

	progress_bar.value = progress * 100.0
	progress_label.text = "Progress: %.1f%% - %.2f MB/s - %d peers - %s" % [
		progress * 100.0, download_rate, num_peers, state
	]

	if state == "finished" or state == "seeding":
		status_label.text = "Download complete!"
		download_button.text = "Download Complete"

func _process_alerts():
	if not session or not session.is_running():
		return

	session.post_torrent_updates() # Request updates from libtorrent
	var alerts = session.get_alerts()
	if not alerts.is_empty():
		print("Processing %d alerts..." % alerts.size())
		for alert in alerts:
			print("  Alert type: %s" % alert.get("type", "unknown"))

func _on_metadata_received(info_hash: String):
	print("Metadata received for info_hash: ", info_hash)
	# Check if the received metadata is for the current handle
	if current_handle and current_handle.is_valid() and current_handle.get_info_hash() == info_hash:
		_display_torrent_info()
		# Now that we have the metadata, pause the torrent to let the user choose files.
		# The download will start when the user clicks the "Download" button, which calls resume().
		current_handle.pause()
	else:
		status_label.text = "Received metadata for an unknown torrent."

func _notification(what):
	if what == NOTIFICATION_WM_CLOSE_REQUEST:
		if session:
			session.stop_session()
