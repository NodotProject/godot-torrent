extends Control

# P2P File Sharing Example
# Demonstrates creating torrents from local files and sharing them

var session: TorrentSession
var shared_files: Dictionary = {}  # torrent_name -> handle

@onready var file_selector: FileDialog = $FileDialog
@onready var share_button: Button = $VBoxContainer/ShareSection/ShareButton
@onready var file_path_input: LineEdit = $VBoxContainer/ShareSection/FilePathInput
@onready var browse_button: Button = $VBoxContainer/ShareSection/BrowseButton
@onready var tracker_input: LineEdit = $VBoxContainer/ShareSection/TrackerInput
@onready var shared_list: ItemList = $VBoxContainer/SharedList
@onready var magnet_output: TextEdit = $VBoxContainer/MagnetSection/MagnetOutput
@onready var copy_button: Button = $VBoxContainer/MagnetSection/CopyButton
@onready var status_label: Label = $VBoxContainer/StatusLabel

func _ready():
	# Initialize session
	session = TorrentSession.new()
	session.start_session()

	# Enable DHT for better peer discovery
	var settings = {
		"enable_dht": true,
		"enable_lsd": true,  # Local Service Discovery
	}
	session.apply_settings(settings)

	# Connect signals
	share_button.pressed.connect(_on_share_button_pressed)
	browse_button.pressed.connect(_on_browse_button_pressed)
	copy_button.pressed.connect(_on_copy_button_pressed)
	shared_list.item_selected.connect(_on_shared_item_selected)

	# Setup file dialog
	file_selector.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	file_selector.file_selected.connect(_on_file_selected)

	# Set default tracker
	tracker_input.text = "udp://tracker.opentrackr.org:1337/announce"

	# Start update timer
	var timer = Timer.new()
	timer.wait_time = 2.0
	timer.timeout.connect(_update_shared_files)
	add_child(timer)
	timer.start()

	status_label.text = "Session started with DHT. Ready to share files."

func _on_browse_button_pressed():
	file_selector.popup_centered_ratio(0.7)

func _on_file_selected(path: String):
	file_path_input.text = path

func _on_share_button_pressed():
	var file_path = file_path_input.text.strip_edges()
	var tracker_url = tracker_input.text.strip_edges()

	if file_path.is_empty():
		status_label.text = "Error: Please select a file to share"
		return

	if not FileAccess.file_exists(file_path):
		status_label.text = "Error: File does not exist"
		return

	# NOTE: TorrentCreator class would be used here in full implementation
	# For now, we demonstrate sharing existing .torrent files

	status_label.text = "Creating torrent (simulated)..."

	# In a real implementation, this would:
	# 1. Create .torrent file using TorrentCreator
	# 2. Add the torrent in seed mode
	# 3. Generate magnet URI

	# Simulated torrent creation
	var file_name = file_path.get_file()
	var torrent_name = file_name + ".torrent"

	# For demonstration, we'll add the file's directory as the torrent
	var directory = file_path.get_base_dir()

	# Create a fake magnet for demonstration
	# In real implementation, use TorrentCreator to generate proper torrent
	var info_hash = _generate_info_hash(file_path)
	var magnet_uri = "magnet:?xt=urn:btih:%s&dn=%s&tr=%s" % [
		info_hash,
		file_name.uri_encode(),
		tracker_url.uri_encode()
	]

	# Add to shared list
	var list_idx = shared_list.add_item("%s [Sharing]" % file_name)
	shared_files[file_name] = {
		"path": file_path,
		"magnet": magnet_uri,
		"list_item": list_idx,
		"handle": null
	}

	status_label.text = "Sharing: %s" % file_name

func _generate_info_hash(file_path: String) -> String:
	# Generate a deterministic hash from file path
	# In real implementation, this would be calculated from torrent metadata
	var hash = file_path.hash()
	return "%040x" % hash  # Format as 40-character hex string

func _on_shared_item_selected(index: int):
	# Find the shared file for this list item
	for file_name in shared_files.keys():
		var file_data = shared_files[file_name]
		if file_data["list_item"] == index:
			magnet_output.text = file_data["magnet"]
			copy_button.disabled = false
			break

func _on_copy_button_pressed():
	var magnet = magnet_output.text
	DisplayServer.clipboard_set(magnet)
	status_label.text = "Magnet URI copied to clipboard!"

func _update_shared_files():
	for file_name in shared_files.keys():
		var file_data = shared_files[file_name]
		var list_idx = file_data["list_item"]
		var handle = file_data["handle"]

		if handle and handle.is_valid():
			var status = handle.get_status()
			if status:
				var upload_rate = status.get_upload_rate() / 1024.0 / 1024.0
				var num_peers = status.get_num_peers()
				var total_uploaded = status.get_total_upload() / 1024.0 / 1024.0

				var text = "%s [Sharing] - U: %.2f MB/s - %d peers - %.2f MB uploaded" % [
					file_name, upload_rate, num_peers, total_uploaded
				]
				shared_list.set_item_text(list_idx, text)

func _notification(what):
	if what == NOTIFICATION_WM_CLOSE_REQUEST:
		if session:
			session.stop_session()
