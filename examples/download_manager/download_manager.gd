extends Control

# Simple Download Manager Example
# Demonstrates basic torrent downloading with progress tracking and UI

var session: TorrentSession
var active_torrents: Dictionary = {}  # info_hash -> {handle, list_item}

@onready var torrent_list: ItemList = $VBoxContainer/TorrentList
@onready var magnet_input: LineEdit = $VBoxContainer/InputSection/MagnetInput
@onready var download_path_input: LineEdit = $VBoxContainer/InputSection/DownloadPathInput
@onready var add_button: Button = $VBoxContainer/InputSection/AddButton
@onready var status_label: Label = $VBoxContainer/StatusLabel

func _ready():
	# Initialize session
	session = TorrentSession.new()
	session.start_session()
	
	# Enable DHT for magnet link support
	session.start_dht()

	# Set default download path
	download_path_input.text = OS.get_user_data_dir() + "/downloads"

	# Connect signals
	add_button.pressed.connect(_on_add_button_pressed)
	torrent_list.item_selected.connect(_on_torrent_selected)

	# Start update timer
	var timer = Timer.new()
	timer.wait_time = 1.0
	timer.timeout.connect(_update_torrents)
	add_child(timer)
	timer.start()

	status_label.text = "Session started. Ready to download."

func _on_add_button_pressed():
	var magnet_uri = magnet_input.text.strip_edges()
	var download_path = download_path_input.text.strip_edges()

	if magnet_uri.is_empty():
		status_label.text = "Error: Please enter a magnet URI"
		return

	if not magnet_uri.begins_with("magnet:"):
		status_label.text = "Error: Invalid magnet URI format"
		return

	# Create download directory if it doesn't exist
	if not DirAccess.dir_exists_absolute(download_path):
		DirAccess.make_dir_recursive_absolute(download_path)

	# Add torrent
	var handle = session.add_magnet_uri(magnet_uri, download_path)

	if handle and handle.is_valid():
		var info_hash = handle.get_info_hash()
		var list_idx = torrent_list.add_item("Loading... [0%]")

		active_torrents[info_hash] = {
			"handle": handle,
			"list_item": list_idx
		}

		status_label.text = "Torrent added successfully"
		magnet_input.text = ""
	else:
		status_label.text = "Error: Failed to add torrent"

func _update_torrents():
	# Request status updates via alerts (non-blocking)
	session.post_torrent_updates()
	
	# Get alerts (also non-blocking)
	var alerts = session.get_alerts()
	
	for alert in alerts:
		# Check if this alert contains torrent status
		if alert.has("torrent_status"):
			var statuses = alert["torrent_status"]
			for status_dict in statuses:
				var info_hash = status_dict.get("info_hash", "")
				
				# Skip if this torrent is not in our list
				if not active_torrents.has(info_hash):
					continue
				
				var torrent_data = active_torrents[info_hash]
				var handle = torrent_data["handle"]
				var list_idx = torrent_data["list_item"]
				
				if not handle.is_valid():
					continue
				
				# Get status info from alert
				var progress = status_dict.get("progress", 0.0) * 100.0
				var download_rate = status_dict.get("download_rate", 0) / 1024.0 / 1024.0  # MB/s
				var upload_rate = status_dict.get("upload_rate", 0) / 1024.0 / 1024.0  # MB/s
				var num_peers = status_dict.get("num_peers", 0)
				var state = status_dict.get("state", 0)
				var state_str = _get_state_string(state)
				
				# Get torrent name
				var info = handle.get_torrent_info()
				var name = info.get_name() if info and info.is_valid() else "Unknown"
				
				# Update list item
				var text = "%s [%.1f%%] - D: %.2f MB/s U: %.2f MB/s - %d peers - %s" % [
					name, progress, download_rate, upload_rate, num_peers, state_str
				]
				torrent_list.set_item_text(list_idx, text)
				
				# Update status label if this is a selected torrent
				if torrent_list.is_selected(list_idx):
					_update_status_for_handle_with_dict(handle, status_dict)

func _on_torrent_selected(index: int):
	# Find the handle for this list item
	for info_hash in active_torrents.keys():
		var torrent_data = active_torrents[info_hash]
		if torrent_data["list_item"] == index:
			# Trigger an immediate update for this torrent
			session.post_torrent_updates()
			break

func _get_state_string(state: int) -> String:
	match state:
		0: return "Queued for checking"
		1: return "Checking files"
		2: return "Downloading metadata"
		3: return "Downloading"
		4: return "Finished"
		5: return "Seeding"
		6: return "Allocating"
		7: return "Checking resume data"
		_: return "Unknown"

func _update_status_for_handle_with_dict(handle, status_dict):
	var info = handle.get_torrent_info()
	var name = info.get_name() if info and info.is_valid() else "Unknown"
	var total_size = info.get_total_size() if info and info.is_valid() else 0

	var progress = status_dict.get("progress", 0.0) * 100.0
	var downloaded = status_dict.get("total_download", 0)
	var uploaded = status_dict.get("total_upload", 0)
	var download_rate = status_dict.get("download_rate", 0) / 1024.0 / 1024.0
	var upload_rate = status_dict.get("upload_rate", 0) / 1024.0 / 1024.0
	var num_peers = status_dict.get("num_peers", 0)
	var num_seeds = status_dict.get("num_seeds", 0)
	var state = status_dict.get("state", 0)

	status_label.text = """Name: %s
Progress: %.1f%% (%.2f / %.2f MB)
Downloaded: %.2f MB | Uploaded: %.2f MB
Download Rate: %.2f MB/s | Upload Rate: %.2f MB/s
Peers: %d | Seeds: %d
State: %s""" % [
		name,
		progress,
		downloaded / 1024.0 / 1024.0,
		total_size / 1024.0 / 1024.0,
		downloaded / 1024.0 / 1024.0,
		uploaded / 1024.0 / 1024.0,
		download_rate,
		upload_rate,
		num_peers,
		num_seeds,
		_get_state_string(state)
	]

func _notification(what):
	if what == NOTIFICATION_WM_CLOSE_REQUEST:
		# Clean shutdown
		if session:
			for info_hash in active_torrents.keys():
				var handle = active_torrents[info_hash]["handle"]
				if handle and handle.is_valid():
					handle.pause()
			session.stop_session()
